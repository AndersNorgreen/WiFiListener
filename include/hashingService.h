#ifndef HASHING_SERVICE_H
#define HASHING_SERVICE_H

#include <Arduino.h>
#include <mbedtls/md.h>
#include "mbedtls/pkcs5.h"

class HashingService {
  public:
    HashingService() {
      const char* defaultSalt = "Salt!";
      memcpy(salt, defaultSalt, strlen(defaultSalt));

      md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);;
    };

    String hash(const char* textToHash);
    bool verifyHash(char* textToVerify, const String& expectedHash);
    String stringify(const unsigned char* hash, size_t len);

  private:
    static const size_t key_len = 32;
    const int iterations = 10000;
    unsigned char derived_key[key_len];
    unsigned char salt[16] = {};
    const mbedtls_md_info_t* md_info;
};

#endif