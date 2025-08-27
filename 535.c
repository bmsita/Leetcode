#define BASE_URL "http://tinyurl.com/"
#define MAX_URLS 10000

char *storedUrls[MAX_URLS];
int urlCount = 0;

/** Encodes a URL to a shortened URL. */
char *encode(char *longUrl)
{
    if (urlCount >= MAX_URLS)
        return NULL; // storage full

    // store the URL
    storedUrls[urlCount] = strdup(longUrl);

    // create short URL with ID
    char *shortUrl = (char *)malloc(strlen(BASE_URL) + 12);
    sprintf(shortUrl, "%s%d", BASE_URL, urlCount);

    urlCount++;
    return shortUrl;
}

/** Decodes a shortened URL to its original URL. */
char *decode(char *shortUrl)
{
    int id;
    sscanf(shortUrl + strlen(BASE_URL), "%d", &id);

    if (id < 0 || id >= urlCount)
        return NULL;
    return storedUrls[id];
}