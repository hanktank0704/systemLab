#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mystring.h"

int main(int argc, char* argv[])
{
  char filename[100];
  int argv_len = 0;
  for(int i = 0; argv[1][i] != '\0'; i++) {
    argv_len++;
  }
  int filename_len = argv_len + 1;
  filename[0] = '.';
  filename[1] = '/';
  for(int i = 0; i < filename_len; i++) {
    filename[i + 2] = argv[1][i];
  }

  char buffer[BUFF_SIZE];
  for(int i = 0; i < BUFF_SIZE; i++) {
    buffer[i] = '\0';
  }
  int idx = 0;
  while(1) {
    int int_char = getchar();
    if(int_char == -1) {
      break;
    }
    char ch = (char)int_char;
    if(ch == '\n') {
      if(end_of_input(buffer) == 1) {
        break;
      }
      enum input_type type = get_type(buffer, idx);
      if(type == single_word) {
        search_single_word(filename, buffer, idx);
      } else if(type == multiple_words) {
        search_multiple_words(filename, buffer, idx);
      } else if(type == quoted) {
        search_quoted(filename, buffer, idx);
      } else if(type == two_words) {
        search_asterisk(filename, buffer, idx);
      } else { 
        char out[] = "invalid input. try again\n";
        write(1, out, sizeof(out));
      }
      for(int i = 0; i < BUFF_SIZE; i++) {
        buffer[i] = '\0';
      }
      idx = 0;
      continue;
    } else {
      buffer[idx++] = (char)int_char;
    }
  }

  char file_buffer[BUFF_SIZE];
  for(int i = 0; i < BUFF_SIZE; i++) {
    file_buffer[i] = '\0';
  }
  char ch = '\0';
  int fd = 0;
  int file_idx = 0;
  int c = 0;
  return 0;
}
