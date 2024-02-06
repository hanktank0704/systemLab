#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

const int BUFF_SIZE = 200000;
int DEBUG = 0; // Only use for debug

enum input_type {
    invalid = 0, 
    single_word,
    multiple_words,
    quoted,
    two_words
};

int getchar(void) {
   char c;
   if (read(0, &c, 1) == 1)
      return c;
   else return -1;
}

void print_single_digit(int num) {
  if(num == 0) {
    char out[] = "0";
    write(1, out, sizeof(out) - 1);
  } else if(num == 1) {
    char out[] = "1";
    write(1, out, sizeof(out) - 1);
  } else if(num == 2) {
    char out[] = "2";
    write(1, out, sizeof(out) - 1);
  } else if(num == 3) {
    char out[] = "3";
    write(1, out, sizeof(out) - 1);
  } else if(num == 4) {
    char out[] = "4";
    write(1, out, sizeof(out) - 1);
  } else if(num == 5) {
    char out[] = "5";
    write(1, out, sizeof(out) - 1);
  } else if(num == 6) {
    char out[] = "6";
    write(1, out, sizeof(out) - 1);
  } else if(num == 7) {
    char out[] = "7";
    write(1, out, sizeof(out) - 1);
  } else if(num == 8) {
    char out[] = "8";
    write(1, out, sizeof(out) - 1);
  } else if(num == 9) {
    char out[] = "9";
    write(1, out, sizeof(out) - 1);
  } 
}

void print_space() {
  char out[] = " ";
  write(1, out, sizeof(out) - 1);
}

void print_colon() {
  char out[] = ":";
  write(1, out, sizeof(out) - 1);
}

void print_line_break() {
  char out[] = "\n";
  write(1, out, sizeof(out) - 1);
}

void print_file_open_error() {
  char out[] = "failed to open file! please check the file path\n";
  write(1, out, sizeof(out) - 1);  
}

void print_number(int num) {
  if(num == 0) {
    print_single_digit(0);
    return;
  }
  int digit = 0;
  int exp = 1;
  while(exp <= num) {
    exp *= 10;
    digit++;
  }
  exp /= 10;
  digit--;
  while(exp != 0) {
    int n = num / exp;
    print_single_digit(n);
    num -= exp * n;
    exp /= 10;
  }
}

int is_character(char ch) {
  return (33 <= (int)ch && (int)ch <= 126);
}
int is_alphabet(char ch) {
  int cval = (int)ch;
  return (65 <= cval && cval <= 90) || (97 <= cval && cval <= 122);
}
int is_alphabet_or_number(char ch) {
  int cval = (int)ch;
  return (48 <= cval && cval < 58) || (65 <= cval && cval <= 90) || (97 <= cval && cval <= 122);
}

int is_line_break(char ch) {
  return (ch == '\n') || (ch == '\r');
}

int abs(int val) {
  return (val >= 0) ? val : -val;
}

int has_quote(char str[], int len) {
  for(int i = 0; i < len; i++) {
    if(str[i] == '"') return 1;
  } 
  return 0;
}

int has_asterisk(char str[], int len) {
  for(int i = 0; i < len; i++) {
    if(str[i] == '*') return 1;
  } 
  return 0;
}

int has_blank(char str[], int len) {
  for(int i = 0; i < len; i++) {
    if(str[i] == ' ') return 1;
  } 
  return 0;
}

int get_type(char str[], int len) {
  enum input_type type;
  int quote = has_quote(str, len); 
  int asterisk = has_asterisk(str, len); 
  int blank = has_blank(str, len);
  if(quote == 1 && asterisk == 1) type = invalid;
  else {
    if(quote == 1) type = quoted;
    else {
      if(asterisk == 1) type = two_words;
      else if(blank == 1) {
        type = multiple_words;
      } 
      else {
        type = single_word;
      }
    }
  } 
  return type;
}

int end_of_input(char str[]) {
  if(str[0] != 'P') return 0;
  if(str[1] != 'A') return 0;
  if(str[2] != '1') return 0;
  if(str[3] != 'E') return 0;
  if(str[4] != 'X') return 0;
  if(str[5] != 'I') return 0;
  if(str[6] != 'T') return 0;
  return 1;
}

int str_cmp(char input_buffer[], int input_buffer_start, int input_buffer_end, char file_buffer[], int file_buffer_start, int file_buffer_end) {
  int input_buffer_len = input_buffer_end - input_buffer_start;
  int file_buffer_len = file_buffer_end - file_buffer_start;
  if(input_buffer_len != file_buffer_len) return 0;
  for(int i = 0; i < input_buffer_len; i++) {
    char input_c = input_buffer[input_buffer_start + i];
    char file_c = file_buffer[file_buffer_start + i];
    if((input_c != file_c) && !((is_alphabet(input_c) && is_alphabet(file_c) && abs(input_c - file_c) == 32))) {
      return 0;
    }
  }
  return 1;
}

int str_find(char string_buffer[], int string_buffer_start, int string_buffer_end, char word_buffer[], int word_buffer_start, int word_buffer_end) {
  int string_buffer_len = string_buffer_end - string_buffer_start;
  int word_buffer_len = word_buffer_end - word_buffer_start;
  for(int i = string_buffer_start; i + word_buffer_len - 1 < string_buffer_end; i++) {
    if(i - 1 >= string_buffer_start && string_buffer[i - 1] != ' ') {
      continue;
    }
    if(i + word_buffer_len < string_buffer_end && string_buffer[i + word_buffer_len] != ' ') {
      continue;
    }
    int same = 1;
    for(int j = 0; j < word_buffer_len; j++) {
      char string_c = string_buffer[i + j];
      char word_c = word_buffer[word_buffer_start + j];
      if((string_c != word_c) && !(is_alphabet(string_c) && is_alphabet(word_c) && abs(string_c - word_c) == 32)) {
        same = 0; 
        break;
      }
    }
    if(same == 1) return i;
  }
  return -1;
}

int str_find_backward(char string_buffer[], int string_buffer_start, int string_buffer_end, char word_buffer[], int word_buffer_start, int word_buffer_end) {
  int string_buffer_len = string_buffer_end - string_buffer_start;
  int word_buffer_len = word_buffer_end - word_buffer_start;
  for(int i = string_buffer_end - word_buffer_len; i >= string_buffer_start; i--) {
    if(i - 1 >= string_buffer_start && string_buffer[i - 1] != ' ') {
      continue;
    }
    if(i + word_buffer_len < string_buffer_end && string_buffer[i + word_buffer_len] != ' ') {
      continue;
    }
    int same = 1;
    for(int j = 0; j < word_buffer_len; j++) {
      char string_c = string_buffer[i + j];
      char word_c = word_buffer[word_buffer_start + j];
      if((string_c != word_c) && !(is_alphabet(string_c) && is_alphabet(word_c) && abs(string_c - word_c) == 32)) {
        same = 0; 
        break;
      }
    }
    if(same == 1) return i;
  }
  return -1;
}

int str_find_quoted(char string_buffer[], int string_buffer_start, int string_buffer_end, char word_buffer[], int word_buffer_start, int word_buffer_end) {
  int string_buffer_len = string_buffer_end - string_buffer_start;
  int word_buffer_len = word_buffer_end - word_buffer_start;
  for(int i = string_buffer_start; i + word_buffer_len - 1 < string_buffer_end; i++) {
    if(i - 1 >= string_buffer_start && string_buffer[i - 1] != ' ') {
      continue;
    }
    if(i + word_buffer_len < string_buffer_end && string_buffer[i + word_buffer_len] != ' ') {
      continue;
    }
    int same = 1;
    for(int j = 0; j < word_buffer_len; j++) {
      char string_c = string_buffer[i + j];
      char word_c = word_buffer[j];
      if((string_c != word_c) && !(is_alphabet(string_c) && is_alphabet(word_c) && abs(string_c - word_c) == 32)) {
        same = 0; 
        break;
      }
    }
    if(same == 1) return 1;
  }
  return 0;
}

void search_single_word(char filename[], char buffer[], int len) {
  char file_buffer[BUFF_SIZE];
  for(int i = 0; i < BUFF_SIZE; i++) {
    file_buffer[i] = '\0';
  }
  char ch = '\0';
  int fd;
  int file_idx = 0;
  int c = 0;
  int cur_line_num = 1;
  int cur_word_pos = 0;
  int line_num = 1;
  int word_pos = 0;
  int first_output = 0;
  if (0 < ( fd = open(filename, O_RDONLY))){
    while(1) {
      if(read(fd, &ch, 1) == 1) {
        if(is_line_break(ch) == 1) {
          int same = str_cmp(buffer, 0, len, file_buffer, 0, file_idx);
          if(same == 1) {
            if(first_output == 1) {
              print_space();
            } else {
              first_output = 1;
            }
            print_number(line_num);
            print_colon();
            print_number(word_pos);
          }
          line_num = ++cur_line_num;
          word_pos = 0;
          cur_word_pos = 0;
          file_idx = 0;
          continue;
        } else if(is_character(ch) != 1) {
          int same = str_cmp(buffer, 0, len, file_buffer, 0, file_idx);
          if(same == 1) {
            if(first_output == 1) {
              print_space();
            } else {
              first_output = 1;
            }
            print_number(line_num);
            print_colon();
            print_number(word_pos);
          }
          line_num = cur_line_num;
          word_pos = ++cur_word_pos;
          file_idx = 0;
        } else {
          file_buffer[file_idx++] = ch;
          cur_word_pos++;
        }
      } else {
        int same = str_cmp(buffer, 0, len, file_buffer, 0, file_idx);
        if(same == 1) {
          if(first_output == 1) {
            print_space();
          } else {
            first_output = 1;
          }
          print_number(line_num);
          print_colon();
          print_number(word_pos);
        }
        break;
      }
    }
    print_line_break();
  } else {
    print_file_open_error();
  }
}

void search_multiple_words(char filename[], char buffer[], int len) {
  int word_idx = 0;
  char file_buffer[BUFF_SIZE];
  char input_word_buffer[BUFF_SIZE];
  char file_word_buffer[BUFF_SIZE];
  int first_output = 0;
  for(int i = 0; i < BUFF_SIZE; i++) {
    file_buffer[i] = '\0';
  }
  char ch = '\0';
  int fd;
  int file_idx = 0;
  int c = 0;
  int cur_line_num = 1;
  int cur_word_pos = 0;
  int line_num = 1;
  int word_pos = 0;

  if (0 < ( fd = open(filename, O_RDONLY))){
    while(1) {
      if(read(fd, &ch, 1) == 1) {
        if(is_line_break(ch) == 1) {
          int include_all_word = 1;
          for(int i = 0; i < len; i++) {
            if(buffer[i] == ' ' || i == len - 1) {
              if(i == len - 1) {
                input_word_buffer[word_idx++] = buffer[i];
              }
              int idx = str_find(file_buffer, 0, file_idx, input_word_buffer, 0, word_idx);
              word_idx = 0;
              if(idx == -1) {
                include_all_word = 0;
                break;
              }
            } else {
              input_word_buffer[word_idx++] = buffer[i];
            }
          }
          if(include_all_word == 1) {
            if(first_output == 1) {
              print_space();
            } else {
              first_output = 1;
            }
            print_number(line_num);
          }
          file_idx = 0;
          line_num++;
        } else {
          file_buffer[file_idx++] = ch;
        }
      } else {
        int include_all_word = 1;
        for(int i = 0; i < len; i++) {
          if(buffer[i] == ' ' || i == len - 1) {
            if(i == len - 1) {
              input_word_buffer[word_idx++] = buffer[i];
            }
            int idx = str_find(file_buffer, 0, file_idx, input_word_buffer, 0, word_idx);
            word_idx = 0;
            if(idx == -1) {
              include_all_word = 0;
              break;
            }
          } else {
            input_word_buffer[word_idx++] = buffer[i];
          }
        }
        if(include_all_word == 1) {
          if(first_output == 1) {
            print_space();
          } else {
            first_output = 1;
          }
          print_number(line_num);
        }
        break;
      }
    }
    print_line_break();
  } else {
    print_file_open_error();
  } 
}

void search_quoted(char filename[], char buffer[], int len) {
  int word_idx = 0;
  char file_buffer[BUFF_SIZE];
  char input_word_buffer[BUFF_SIZE];
  char file_word_buffer[BUFF_SIZE];
  int first_output = 0;
  for(int i = 0; i < BUFF_SIZE; i++) {
    file_buffer[i] = '\0';
  }
  char ch = '\0';
  int fd;
  int file_idx = 0;
  int c = 0;
  int cur_line_num = 1;
  int cur_word_pos = 0;
  int line_num = 1;
  int word_pos = 0;
  
  if (0 < ( fd = open(filename, O_RDONLY))){
    while(1) {
      if(read(fd, &ch, 1) == 1) {
        if(is_line_break(ch) == 1) {
          int include_all_word = 1;
          int file_buffer_start = 0;
          int file_buffer_end = file_idx;
          while(1) {
            int idx = str_find(file_buffer, file_buffer_start, file_buffer_end, buffer, 1, len - 1);
            if(idx == -1) {
              break;
            }  
            if(first_output == 1) {
              print_space();
            } else {
              first_output = 1;
            }
            print_number(line_num);
            print_colon();
            print_number(idx);
            file_buffer_start = idx + 1;
          }
          file_idx = 0;
          word_pos = 0;
          line_num++;
        } else {
          file_buffer[file_idx++] = ch;
          word_pos++;
        }
      } else {
        int include_all_word = 1;
        int idx = str_find(file_buffer, 0, file_idx, buffer, 1, len - 1);
        if(idx != -1) {
          if(first_output == 1) {
            print_space();
          } else {
            first_output = 1;
          }
          print_number(line_num);
          print_colon();
          print_number(idx);
        }
        break;
      }
    }
    print_line_break();
  } else {
    print_file_open_error();
  } 
}

void search_asterisk(char filename[], char buffer[], int len) {
  int word_idx = 0;
  char file_buffer[BUFF_SIZE];
  char input_word_buffer[BUFF_SIZE];
  char file_word_buffer[BUFF_SIZE];
  for(int i = 0; i < BUFF_SIZE; i++) {
    file_buffer[i] = '\0';
  }
  char ch = '\0';
  int fd;
  int file_idx = 0;
  int c = 0;
  int cur_line_num = 1;
  int cur_word_pos = 0;
  int line_num = 1;
  int word_pos = 0;
  int first_output = 0;
  if (0 < ( fd = open(filename, O_RDONLY))){
    while(1) {
      if(read(fd, &ch, 1) == 1) {
        if(is_line_break(ch) == 1) {
          int include_all_word = 1;
          int asterisk_pos = 0;
          for(int i = 0; i < len; i++) {
            if(buffer[i] == '*') {
              asterisk_pos = i;
              break;
            }
          }
          int idx1 = str_find(file_buffer, 0, file_idx, buffer, 0, asterisk_pos);
          int idx2 = str_find_backward(file_buffer, 0, file_idx, buffer, asterisk_pos + 1, len);
          int character_exist = 0;
          if(idx1 == -1 || idx2 == -1) {
            file_idx = 0;
            word_pos = 0;
            line_num++;
            continue;
          }
          for(int i = idx1 + asterisk_pos; i < idx2; i++) {
            if(is_character(file_buffer[i])) {
              character_exist = 1;
              break;
            }
          }
          if(character_exist == 1) {
            if(first_output == 1) {
              print_space();
            } else {
              first_output = 1;
            }
            print_number(line_num);
          }
          file_idx = 0;
          word_pos = 0;
          line_num++;
        } else {
          file_buffer[file_idx++] = ch;
          word_pos++;
        }
      } else {
        int include_all_word = 1;
        int asterisk_pos = 0;
        for(int i = 0; i < len; i++) {
          if(buffer[i] == '*') {
            asterisk_pos = i;
            break;
          }
        }
        int idx1 = str_find(file_buffer, 0, file_idx, buffer, 0, asterisk_pos);
        int idx2 = str_find_backward(file_buffer, 0, file_idx, buffer, asterisk_pos + 1, len);
        int character_exist = 0;
        if(idx1 == -1 || idx2 == -1) {
          break;
        }
        for(int i = idx1 + asterisk_pos; i < idx2; i++) {
          if(is_character(file_buffer[i])) {
            character_exist = 1;
            break;
          }
        }
        if(character_exist == 1) {
          if(first_output == 1) {
            print_space();
          } else {
            first_output = 1;
          }
          print_number(line_num);
        }
        return;
      }
    }
    print_line_break();
  } else {
    print_file_open_error();
  } 
}
