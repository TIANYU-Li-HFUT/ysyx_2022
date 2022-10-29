#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
 // panic("Not implemented");
  int count = 0;
  while (*s)
  {
    count++;
    s++;
  }
  return count;
}

char *strcpy(char *dst, const char *src) {
  //panic("Not implemented");
  assert(dst && src);
  char* ret = dst;
  while ((*(dst++) = *(src++)))
  {
    ;
  }
  return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
  //panic("Not implemented");
  char* start = dst; // 记录目标字符串起始位置
  while (n ) // 拷贝字符串
  {
    *(dst++) = *(src++);
    n--;
  }
  if (n) 
  {
    while (--n)
    {
      *dst++ = '\0';
    }
  }
  return start;
}

char *strcat(char *dst, const char *src) {
  //panic("Not implemented");
  assert(dst && src);
  char* ret = dst;
  while (*dst){
    dst++;
  }
  while ((*(dst++) = *(src++)))
  {
    ;
  }
  return ret;
}

int strcmp(const char *s1, const char *s2) {
  //panic("Not implemented");
  assert(s1 && s2);
  while (*s1 == *s2)
  {
    if (*s1 == '\0')
      return 0;
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  //panic("Not implemented");
    assert(s1 != NULL && s2 != NULL);
    int ret = 0;
    while (n-- != 0)
    {
        ret = *s1 - *s2;
        if (ret != 0)
            break;
        s1++;
        s2++;
    }
    return ret;
}

void *memset(void *s, int c, size_t n) {
  //panic("Not implemented");
    assert( s != NULL && n >= 0);
    char* p = (char*)s;
 
    unsigned  int i = 0;
    while (i < n)
    {
        p[i++] = c;
    }
    return (void*)p;
}

void *memmove(void *dst, const void *src, size_t n) {
  //panic("Not implemented");
  char *ret=(char *)dst;
  char *dest_t=dst;
  char *src_t=(char *)src;
  assert( NULL !=src && NULL !=dst);
  
  if (dest_t<=src_t || dest_t>=src_t+n)
  {
        while(n--)
    {
      *dest_t++ = *src_t++;
    }
  }
  else
  {
    dest_t+=n-1;
    src_t+=n-1;
    while(n--)
    {
      *dest_t--=*src_t--;
    }
  }
  return ret;
}

void *memcpy(void *out, const void *in, size_t n) {  //重叠情况要用memmove
  //panic("Not implemented");
 assert(out && in);
 void* ret = out;
 while (n--)//要拷贝的字节为0跳出循环
 {
   *(char*)out = *(char*)in;
   //强制类型转化为char*类型，每次指针加一跳过一个字节，解引用访问一个字节，精度最细，方便应对各种类型的元素的字节的拷贝
   //dest = ((char*)dest)++;
   //src = ((char*)src)++;
   //这种写法实质上是：强制类型转化时创建了一个临时变量，对其解引用无意义，c++不支持这种写法，建议写成下列写法
   out = (char*)out + 1;
   in = (char*)in + 1;
 }
 return ret;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  //panic("Not implemented");
  assert(s1 != NULL);
  assert(s2 != NULL);
 
  while (n--)
  {
        //需要强制类型转换成char类型，进行字节位比较
    if (*(char*)s1 != *(char*)s2)    //如果该字节数字不同，返回其差值
    {
      return *(char*)s1 - *(char*)s2;
    }
    s1 = (char*)s1 + 1;              
    s2 = (char*)s2 + 1;
  }
  return 0;   

}

#endif

 
 
