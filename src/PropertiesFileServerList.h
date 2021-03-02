//
// Created by stephane bourque on 2021-02-12.
//

#ifndef UCENTRAL_PROPERTIESFILESERVERLIST_H
#define UCENTRAL_PROPERTIESFILESERVERLIST_H

#include <string>
#include <memory>
#include <vector>
#include <iterator>

class PropertiesFileServerEntry {
public:
    PropertiesFileServerEntry(const std::string &address,
                               const uint32_t port,
                               const std::string &key_file,
                               const std::string &cert_file,
                               const std::string &key_file_password = "" ) :
                               address_(address),
                               port_(port),
                               key_file_(key_file),
                               cert_file_(cert_file),
                               key_file_password_(key_file_password) {};

    const std::string & address() const { return address_; };
    const uint32_t port() const { return port_; };
    const std::string & key_file() const { return key_file_; };
    const std::string & cert_file() const { return cert_file_; };
    const std::string & key_file_password() const { return key_file_password_; };

private:
    std::string     address_;
    std::string     cert_file_;
    std::string     key_file_;
    std::string     key_file_password_;
    uint32_t        port_;
};

class PropertiesFileServerList {
public:
    PropertiesFileServerList(const std::string &prefix):prefix_(prefix) {};

    void initialize();

    const PropertiesFileServerEntry & operator[](int);
    inline std::vector<PropertiesFileServerEntry>::iterator begin() noexcept { return list_.begin(); };
    inline std::vector<PropertiesFileServerEntry>::iterator end() noexcept { return list_.end(); };
    inline std::vector<PropertiesFileServerEntry>::const_iterator cbegin() const noexcept { return list_.cbegin(); };
    inline std::vector<PropertiesFileServerEntry>::const_iterator cend() const noexcept { return list_.cend(); };
    const uint32_t  size() const { return list_.size(); };

private:
    std::string prefix_;
    std::vector<PropertiesFileServerEntry>   list_;
};

#endif //UCENTRAL_PROPERTIESFILESERVERLIST_H
