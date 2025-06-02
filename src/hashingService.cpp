#include <hashingService.h>

String HashingService::hash(const char* textToHash) {
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, md_info, 1);  // 1 = HMAC - kan sættes til nul, hvis MQTT TLS virker
  
  int ret = mbedtls_pkcs5_pbkdf2_hmac(
    &ctx,
    (const unsigned char*)textToHash,
    strlen(textToHash),
    salt,
    sizeof(salt),
    iterations,
    key_len,
    derived_key);
  
  mbedtls_md_free(&ctx);
  
  if(ret != 0)
    return "Hashing failed";

  return stringify(derived_key, key_len);
}

bool HashingService::verifyHash(char* textToVerify, const String& expectedHash) {
  String hashedInput = hash(textToVerify);
  return hashedInput == expectedHash;
}

String HashingService::stringify(const unsigned char* hash, size_t len) {
  String stringifiedHash;
  char buf[3];
  for (size_t i = 0; i < len; i++) {
    sprintf(buf, "%02x", hash[i]);
    stringifiedHash += buf;
  }

  return stringifiedHash;
}