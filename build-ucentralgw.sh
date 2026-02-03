#!/bin/bash

# Script to build wlan-cloud-ucentralgw project in a Debian Bookworm container
# Dependencies are built in a temporary folder within the repo

set -e

# Configuration
POCO_VERSION="poco-tip-v4-tag"
CPPKAFKA_VERSION="tip-v1"
VALIJSON_VERSION="tip-v1.0.2"
DEBIAN_VERSION="bookworm"
TEMP_BUILD_DIR=".build-deps"
OUTPUT_DIR="./cmake-build"
BINARY_NAME="owgw"
CONTAINER_NAME="ucentralgw-builder-$$"
IMAGE_NAME="ucentralgw-build-env:${DEBIAN_VERSION}"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Building wlan-cloud-ucentralgw in Debian ${DEBIAN_VERSION} container ===${NC}"

# Check if Docker is available
if ! command -v docker >/dev/null 2>&1; then
    echo -e "${RED}Error: Docker is not installed or not in PATH${NC}"
    exit 1
fi

# Create temporary build directory structure
echo -e "${YELLOW}Creating temporary build directory: ${TEMP_BUILD_DIR}${NC}"
mkdir -p "${TEMP_BUILD_DIR}"
mkdir -p "${OUTPUT_DIR}"

# Create Dockerfile for build environment
echo -e "${YELLOW}Creating build environment Dockerfile...${NC}"
cat > "${TEMP_BUILD_DIR}/Dockerfile.build" <<EOF
FROM debian:${DEBIAN_VERSION}

# Install build dependencies
RUN apt-get update && apt-get install --no-install-recommends -y \\
    make cmake g++ git \\
    libpq-dev libmariadb-dev libmariadbclient-dev-compat \\
    librdkafka-dev libboost-all-dev libssl-dev \\
    zlib1g-dev nlohmann-json3-dev ca-certificates libfmt-dev

# Set working directory
WORKDIR /build

# Copy build script into container
COPY build-in-container.sh /build/
RUN chmod +x /build/build-in-container.sh

# Entry point
ENTRYPOINT ["/build/build-in-container.sh"]
EOF

# Create the build script that runs inside the container
echo -e "${YELLOW}Creating in-container build script...${NC}"
cat > "${TEMP_BUILD_DIR}/build-in-container.sh" <<'EOFSCRIPT'
#!/bin/bash
set -e

POCO_VERSION="poco-tip-v4-tag"
CPPKAFKA_VERSION="tip-v1"
VALIJSON_VERSION="tip-v1.0.2"
BUILD_DIR="/build/deps"
INSTALL_PREFIX="/build/install"
REPO_DIR="/repo"
OUTPUT_DIR="/repo/cmake-build"

# Number of parallel jobs
JOBS=$(nproc 2>/dev/null || echo 4)

echo "=== Building dependencies in container ==="
echo "Using ${JOBS} parallel jobs"

# Function to build a dependency
build_dependency() {
    local name=$1
    local repo=$2
    local branch=$3
    local build_dir="${BUILD_DIR}/${name}"

    echo "Building ${name}..."

    if [ ! -d "${build_dir}" ]; then
        echo "Cloning ${name} from ${repo} (branch: ${branch})"
        git clone --depth 1 --branch "${branch}" "${repo}" "${build_dir}"
    fi

    mkdir -p "${build_dir}/cmake-build"
    cd "${build_dir}/cmake-build"

    echo "Configuring ${name}..."
    if [ "${name}" = "poco" ]; then
        # POCO needs special configuration to enable all required components
        cmake -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
              -DENABLE_JSON=ON \
              -DENABLE_CRYPTO=ON \
              -DENABLE_JWT=ON \
              -DENABLE_NET=ON \
              -DENABLE_NETSSL=ON \
              -DENABLE_UTIL=ON \
              -DENABLE_DATA=ON \
              -DENABLE_DATA_SQLITE=ON \
              -DENABLE_DATA_POSTGRESQL=ON \
              -DENABLE_DATA_MYSQL=ON \
              ..
    else
        cmake -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" ..
    fi

    echo "Building ${name}..."
    cmake --build . --config Release -j${JOBS}

    echo "Installing ${name}..."
    cmake --build . --target install

    # For POCO, create symlinks if needed and show what was installed
    if [ "${name}" = "poco" ]; then
        echo "POCO libraries installed:"
        find "${INSTALL_PREFIX}/lib" -name "libPoco*" -type f || true

        # Run ldconfig to update library cache
        cd "${INSTALL_PREFIX}/lib"

        # Create missing symlinks if needed
        for lib in libPocoJSON.so libPocoCrypto.so libPocoNet.so libPocoNetSSL.so libPocoUtil.so libPocoFoundation.so libPocoData.so libPocoDataSQLite.so libPocoDataPostgreSQL.so libPocoDataMySQL.so libPocoJWT.so; do
            if [ ! -e "${lib}" ]; then
                # Find versioned library and create symlink
                versioned=$(find . -maxdepth 1 -name "${lib}.*" | head -n 1)
                if [ -n "$versioned" ]; then
                    echo "Creating symlink: ${lib} -> ${versioned}"
                    ln -sf "$(basename ${versioned})" "${lib}"
                fi
            fi
        done
    fi

    echo "${name} built successfully"
}

# Create directories
mkdir -p "${BUILD_DIR}"
mkdir -p "${INSTALL_PREFIX}"

# Build POCO
build_dependency \
    "poco" \
    "https://github.com/Telecominfraproject/wlan-cloud-lib-poco" \
    "${POCO_VERSION}"

# Build cppkafka
build_dependency \
    "cppkafka" \
    "https://github.com/Telecominfraproject/wlan-cloud-lib-cppkafka" \
    "${CPPKAFKA_VERSION}"

# Build valijson
build_dependency \
    "valijson" \
    "https://github.com/Telecominfraproject/wlan-cloud-lib-valijson" \
    "${VALIJSON_VERSION}"

# Build the main application
echo "=== Building owgw ==="

cd "${REPO_DIR}"

# Configure git to trust the repository directory (needed for mounted volumes)
git config --global --add safe.directory /repo

# Clean the build directory to avoid CMake cache conflicts
if [ -d "${OUTPUT_DIR}" ]; then
    echo "Cleaning existing build directory..."
    rm -rf "${OUTPUT_DIR}"
fi

mkdir -p "${OUTPUT_DIR}"
cd "${OUTPUT_DIR}"

echo "Configuring owgw..."
export CMAKE_PREFIX_PATH="${INSTALL_PREFIX}"
export PKG_CONFIG_PATH="${INSTALL_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}"
export LD_LIBRARY_PATH="${INSTALL_PREFIX}/lib:${LD_LIBRARY_PATH}"

# Debug: show what libraries are available
echo "Installed libraries in ${INSTALL_PREFIX}/lib:"
ls -la "${INSTALL_PREFIX}/lib" || true

cmake \
    -DCMAKE_PREFIX_PATH="${INSTALL_PREFIX}" \
    -DCMAKE_LIBRARY_PATH="${INSTALL_PREFIX}/lib" \
    -DPoco_DIR="${INSTALL_PREFIX}/lib/cmake/Poco" \
    -DCppKafka_DIR="${INSTALL_PREFIX}/lib/cmake/CppKafka" \
    ..

echo "Building owgw..."
echo "Library search paths:"
echo "  LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
echo "  CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"

# Build with explicit library path
LIBRARY_PATH="${INSTALL_PREFIX}/lib:${LIBRARY_PATH}" \
cmake --build . --config Release -j${JOBS}

echo "=== Build completed successfully ==="
EOFSCRIPT

# Build the Docker image
echo -e "${YELLOW}Building Docker image (this may take a while on first run)...${NC}"
docker build \
    -t "${IMAGE_NAME}" \
    -f "${TEMP_BUILD_DIR}/Dockerfile.build" \
    "${TEMP_BUILD_DIR}"

# Run the build in the container
echo -e "${YELLOW}Running build in container...${NC}"
docker run --rm \
    --name "${CONTAINER_NAME}" \
    -v "$(pwd):/repo" \
    -v "$(pwd)/${TEMP_BUILD_DIR}/build-cache:/build/deps" \
    "${IMAGE_NAME}"

echo -e "${GREEN}=== Build completed successfully! ===${NC}"
echo -e "${GREEN}Binary location: ${OUTPUT_DIR}/${BINARY_NAME}${NC}"
echo ""
echo "To run the binary, you'll need the dependencies installed or use the container."
echo ""
echo "To clean up build dependencies:"
echo -e "${YELLOW}  rm -rf ${TEMP_BUILD_DIR}${NC}"
echo ""
echo "To remove the Docker image:"
echo -e "${YELLOW}  docker rmi ${IMAGE_NAME}${NC}"
echo ""
echo "Note: The temporary build directory (${TEMP_BUILD_DIR}) is excluded from git."

exit 0
