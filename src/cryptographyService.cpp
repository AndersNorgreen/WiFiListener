#include <cryptographyService.h>

String CryptographyService::hash(const char* input) {
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mdInfo, 1);  // 1 = HMAC - kan sættes til nul, hvis MQTT TLS virker
  
  int ret = mbedtls_pkcs5_pbkdf2_hmac(
    &ctx,
    (const unsigned char*)input,
    strlen(input),
    salt,
    sizeof(salt),
    iterations,
    KEY_LEN,
    key);
  
  mbedtls_md_free(&ctx);
  
  if(ret != 0)
    return "Hashing failed";

  return stringify(key, KEY_LEN);
}

bool CryptographyService::verifyHash(const char* input, const String& match) {
  String hashedInput = hash(input);
  return hashedInput == match;
}

String CryptographyService::encrypt(const String& input) {
  unsigned char iv[IV_LEN];
  generateIV(iv);

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, key, KEY_LEN * 8);

  size_t inputLen = input.length();
  size_t paddedLen = inputLen + (16 - (inputLen % 16));
  unsigned char padded[paddedLen];
  pkcs7Pad((const unsigned char*)input.c_str(), inputLen, padded, paddedLen);

  unsigned char ciphertext[paddedLen];
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, paddedLen, iv, padded, ciphertext);

  // Combine IV + ciphertext
  unsigned char finalOutput[IV_LEN + paddedLen];
  memcpy(finalOutput, iv, IV_LEN);
  memcpy(finalOutput + IV_LEN, ciphertext, paddedLen);

  // Base64 encode
  size_t base64Len;
  unsigned char base64Out[(IV_LEN + paddedLen) * 4 / 3 + 4];
  mbedtls_base64_encode(base64Out, sizeof(base64Out), &base64Len, finalOutput, IV_LEN + paddedLen);
  base64Out[base64Len] = '\0';

  mbedtls_aes_free(&aes);
  return String((char*)base64Out);
}

String CryptographyService::decrypt(const String& encryptedBase64) {
  size_t decodedLen;
  unsigned char decoded[encryptedBase64.length() * 3 / 4 + 1];
  mbedtls_base64_decode(decoded, sizeof(decoded), &decodedLen, (const unsigned char*)encryptedBase64.c_str(), encryptedBase64.length());

  if (decodedLen < IV_LEN) return "Decryption failed: IV too short";

  unsigned char iv[IV_LEN];
  memcpy(iv, decoded, IV_LEN);
  unsigned char* ciphertext = decoded + IV_LEN;
  size_t ciphertextLen = decodedLen - IV_LEN;

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, key, KEY_LEN * 8);

  unsigned char decrypted[ciphertextLen];
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ciphertextLen, iv, ciphertext, decrypted);

  size_t plainLen = ciphertextLen;
  pkcs7Unpad(decrypted, plainLen);
  
  mbedtls_aes_free(&aes);
  char outputBuffer[plainLen + 1];
  memcpy(outputBuffer, decrypted, plainLen);
  outputBuffer[plainLen] = '\0';  // ensure null-termination

  return String(outputBuffer);
}

void CryptographyService::generateIV(unsigned char* iv) {
  for (size_t i = 0; i < IV_LEN; ++i) {
    iv[i] = random(0, 256);
  }
}

void CryptographyService::pkcs7Unpad(unsigned char* data, size_t& len) {
  if (len == 0) return;

  size_t pad = data[len - 1];
  if (pad > 16 || pad > len) return;

  // Verify all pad bytes
  for (size_t i = 0; i < pad; ++i) {
    if (data[len - 1 - i] != pad) return;  // Invalid padding
  }

  len -= pad;
}

void CryptographyService::pkcs7Pad(const unsigned char* input, size_t inputLen, unsigned char* output, size_t& outputLen) {
  size_t padding = 16 - (inputLen % 16);
  memcpy(output, input, inputLen);
  for (size_t i = 0; i < padding; ++i) {
    output[inputLen + i] = padding;
  }
  outputLen = inputLen + padding;
}

void CryptographyService::deriveKey(const char* password) {
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);

  int ret = mbedtls_md_setup(&ctx, mdInfo, 1);  // 1 = HMAC
  if (ret != 0) {
      Serial.println("md_setup failed");
      return;
  }

  ret = mbedtls_pkcs5_pbkdf2_hmac(
      &ctx,
      (const unsigned char*)password, strlen(password),
      salt, sizeof(salt),
      iterations,
      KEY_LEN,
      key
  );

  mbedtls_md_free(&ctx);

  if (ret != 0) {
      Serial.println("Key derivation failed");
  }
}

String CryptographyService::stringify(const unsigned char* input, size_t inputLen) {
  String stringifiedInput;
  char buf[3];
  for (size_t i = 0; i < inputLen; i++) {
    sprintf(buf, "%02x", input[i]);
    stringifiedInput += buf;
  }

  return stringifiedInput;
}