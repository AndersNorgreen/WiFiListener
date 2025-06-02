#ifndef CRYPTOGRAPHY_SERVICE_H
#define CRYPTOGRAPHY_SERVICE_H

#include <Arduino.h>
#include <mbedtls/md.h>
#include <mbedtls/base64.h>
#include <mbedtls/pkcs5.h>
#include "mbedtls/aes.h"

class CryptographyService {
  public:
    CryptographyService() {
      const char* defaultSalt = "Salt!"; // TODO: dynamisk ?
      memcpy(salt, defaultSalt, strlen(defaultSalt));
      mdInfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
      deriveKey("Noegle");
    };

    String hash(const char* input); // Kan også bruges til at genere en nøgle, som ellers kan få sin egen metode
    bool verifyHash(const char* input, const String& match);

    String encrypt(const String& input);
    String decrypt(const String& encryptedBase64);

  private:
    static const size_t KEY_LEN = 32;
    static const size_t IV_LEN = 16;
    const int iterations = 10000;
    unsigned char key[KEY_LEN];
    unsigned char salt[16] = {};
    const mbedtls_md_info_t* mdInfo;

    void generateIV(unsigned char* iv);
    void pkcs7Pad(const unsigned char* input, size_t inputLen, unsigned char* output, size_t& outputLen);
    void pkcs7Unpad(unsigned char* data, size_t& len);
    void deriveKey(const char* password);
    
    String stringify(const unsigned char* input, size_t inputLen);
};

#endif