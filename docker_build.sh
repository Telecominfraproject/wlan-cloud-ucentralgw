USERNAME=arilia
HUBNAME=tip-tip-wlan-cloud-ucentral.jfrog.io
IMAGE_NAME=ucentralgw

echo "Removing docker images before build..."
docker rmi -f $(docker images -a -q)
echo "Building $IMAGE_NAME image..."
docker build --no-cache --tag $IMAGE_NAME .
IMAGE_ID=`docker images -q $IMAGE_NAME`
docker login --username=$USERNAME $HUBNAME
docker tag $IMAGE_ID $HUBNAME/$IMAGE_NAME:latest
echo "Updating $HUBNAME with the latest $IMAGE_NAME image..."
docker push $HUBNAME/$IMAGE_NAME
docker logout $HUBNAME
