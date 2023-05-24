#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long parse_chunk_size(const char *arg, int *status) {
  errno = 0;
  long retval = strtol(arg, NULL, 10);
  if (errno) {
    *status = errno;
  }
  if (retval <= 0) {
    *status = EINVAL;
  }
  return retval;
}

int buffer_expander(char **buffer_inout, size_t *size_inout, size_t required) {
  const double RESIZE_FACTOR = 1.5;
  do {
    size_t new_size = *size_inout * RESIZE_FACTOR;
    char *tmp = realloc(*buffer_inout, new_size * sizeof(char));
    if (!tmp) {
      return errno;
    }
    *size_inout = new_size;
    *buffer_inout = tmp;
  } while (*size_inout < required);
  return 0;
}

int buffer_expander_linear(char **buffer_inout, size_t *size_inout,
                           size_t required) {
  char *tmp = realloc(*buffer_inout, required * sizeof(char));
  if (!tmp) {
    return errno;
  }
  *size_inout = required;
  *buffer_inout = tmp;
  return 0;
}

int read_data(FILE *from, char **into, size_t *capacity, size_t *size,
              int (*allocator)(char **, size_t *, size_t), size_t chunk_size) {
  *size = 0;
  do {
    if (*size + chunk_size > *capacity) {
      if (allocator(into, capacity, *size + chunk_size) != 0) {
        return -1;
      }
    }
    size_t items_read = fread(*into, sizeof(char), chunk_size, from);
    *size += items_read;
    if (ferror(from) != 0) {
      return -1;
    }
  } while (feof(from) == 0);
  return 0;
}

typedef struct buffer_st {
  char *data;
  size_t capacity;
  size_t size;
} buffer_t;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <chunk-size>\n", argv[0]);
    return 1;
  }
  int status_code = 0;
  const size_t CHUNK_SIZE = parse_chunk_size(argv[1], &status_code);
  if (status_code != 0) {
    fprintf(stderr, "Error parsing chunk-size argument: %s\n",
            strerror(status_code));
    return 1;
  }

  buffer_t buffer;
  buffer.capacity = CHUNK_SIZE;
  buffer.size = 0;
  buffer.data = malloc(buffer.capacity * sizeof(char));
  if (!buffer.data) {
    perror("Error allocating memory");
    status_code = 1;
    goto cleanup;
  }

  printf("Capacity = %zu, size = %zu\n", buffer.capacity, buffer.size);
  int retval = read_data(stdin, &buffer.data, &buffer.capacity, &buffer.size,
                         &buffer_expander_linear, CHUNK_SIZE);
  if (retval != 0) {
    perror("Error reading data from stdin");
    status_code = 1;
  } else {
    printf("Capacity = %zu, size = %zu\n", buffer.capacity, buffer.size);
  }

cleanup:
  free(buffer.data);
  return status_code;
}
