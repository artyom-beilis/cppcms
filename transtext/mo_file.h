#ifndef MO_FILE
#define MO_FILE

extern "C" {

void *thr_safe_gettext_load(char const *mo_file);
void thr_safe_gettext_unload(void *ptr);
char const *thr_safe_gettext_text_lookup(void *the_localization_text, char const *s,int std_id);

};

#endif
