#ifndef BASE64_H
#define BASE64_H

#ifdef __cplusplus
extern "C" {
#endif
char* base64_encode(const char* data, int data_len);
char* base64_decode(const char* data, int data_len, int *pLen);
#ifdef __cplusplus
}
#endif
#endif // BASE64_H
