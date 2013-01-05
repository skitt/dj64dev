class StringList {
 public:
  StringList();
  ~StringList();
  int count;
  char *operator[](int);
  void add(const char *);
  void del(const char *);
  int  has(const char *);
  void flush(void);
  void sort(void);
 private:
  int maxc;
  int max;
  char **data;
};
