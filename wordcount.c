#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_THREADS 4

typedef struct
{
    char *data;
    size_t start;
    size_t end;
    
}WordCount;

static long int word_count = 0;  

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  


int is_word_char(char c) {
    return isalnum((unsigned char)c);  
}

void *count_words(void *arg)
{
    WordCount *args = (WordCount *)arg;
    char *data = args->data;
    size_t start = args->start;
    size_t end = args->end;
     int count = 0;
    int in_word = 0;  
    for(size_t i = start;i<end;i++)
    {
        if(is_word_char(data[i]))
        {
            if(!in_word)
            {
              in_word = 1;
              count++;
            }
        }
        else
        {
            in_word = 0;
        }
    }

    pthread_mutex_lock(&mutex);
    word_count += count;
    pthread_mutex_unlock(&mutex);
    return NULL;

}
int main(int argc,char **argv)
{
    if(argc<2)
    {
        fprintf(stderr,"Usage ./a.out [filename]");
        return 1;
    }

    const char *filename = argv[1];

    int fd = open(filename,O_RDONLY);
    if(fd<0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    struct stat st;  
    if(stat(filename,&st)!=0)
    {
        perror("stat");
        close(fd);
        exit(EXIT_FAILURE);
    }

    size_t filesize = st.st_size;

    unsigned char *buffer = malloc(filesize);
    if(!buffer)
    {
        perror("malloc");
        return 1;
    }

    ssize_t bytes_read = read(fd,buffer,filesize);
    if(bytes_read < 0)
    {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);

    pthread_t threads[MAX_THREADS];

    WordCount args[MAX_THREADS];

    size_t chunks = filesize/MAX_THREADS;

    for(int i = 0;i<MAX_THREADS;i++)
    {
        args[i].data = buffer;
        args[i].start = i*chunks;
        if(i == MAX_THREADS-1) args[i].end = filesize;
        else  args[i].end = (i+1)*chunks;

    
          if (args[i].start > 0) {
        while (args[i].start < filesize && is_word_char(buffer[args[i].start])) {
            args[i].start++;
          }
        }

   
      if (args[i].end < filesize) {
        while (args[i].end < filesize && is_word_char(buffer[args[i].end])) {
            args[i].end++;
        }
       }
        pthread_create(&threads[i],NULL,count_words,&args[i]);
       
    }

    for(int i = 0;i<MAX_THREADS;i++)
    {
        pthread_join(threads[i],NULL);
    }

    printf("Total words in file: %ld\n", word_count);
    free(buffer);
    pthread_mutex_destroy(&mutex);
}
