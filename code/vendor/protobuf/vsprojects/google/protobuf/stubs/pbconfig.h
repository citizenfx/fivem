// This replaces the shipped one with this that fixes building on linux

#if _MSC_VER >= 1600
#define GOOGLE_PROTOBUF_HASH_NAMESPACE std
#define GOOGLE_PROTOBUF_HASH_MAP_H <unordered_map>
#define GOOGLE_PROTOBUF_HASH_MAP_CLASS unordered_map
#define GOOGLE_PROTOBUF_HASH_SET_H <unordered_set>
#define GOOGLE_PROTOBUF_HASH_SET_CLASS unordered_set
#elif _MSC_VER >= 1310
#define GOOGLE_PROTOBUF_HASH_NAMESPACE stdext
#define GOOGLE_PROTOBUF_HASH_MAP_H <hash_map>
#define GOOGLE_PROTOBUF_HASH_MAP_CLASS hash_map
#define GOOGLE_PROTOBUF_HASH_SET_H <hash_set>
#define GOOGLE_PROTOBUF_HASH_SET_CLASS hash_set
#else
#define GOOGLE_PROTOBUF_HASH_NAMESPACE std
#define GOOGLE_PROTOBUF_HASH_MAP_H <unordered_map>
#define GOOGLE_PROTOBUF_HASH_MAP_CLASS unordered_map
#define GOOGLE_PROTOBUF_HASH_SET_H <unordered_set>
#define GOOGLE_PROTOBUF_HASH_SET_CLASS unordered_set
#define HAVE_PTHREAD 1
#endif

/* the location of <hash_set> */

/* define if the compiler has hash_map */
#define GOOGLE_PROTOBUF_HAVE_HASH_MAP 1

/* define if the compiler has hash_set */
#define GOOGLE_PROTOBUF_HAVE_HASH_SET 1
