#ifndef MYSQL2_CLIENT_H
#define MYSQL2_CLIENT_H

#ifndef HAVE_RB_THREAD_CALL_WITHOUT_GVL
#ifdef HAVE_RB_THREAD_BLOCKING_REGION

/* emulate rb_thread_call_without_gvl with rb_thread_blocking_region */
#define rb_thread_call_without_gvl(func, data1, ubf, data2) \
  rb_thread_blocking_region((rb_blocking_function_t *)func, data1, ubf, data2)

#else /* ! HAVE_RB_THREAD_BLOCKING_REGION */
/*
 * partial emulation of the 2.0 rb_thread_call_without_gvl under 1.8,
 * this is enough for dealing with blocking I/O functions in the
 * presence of threads.
 */

#include <rubysig.h>
#define RUBY_UBF_IO ((rb_unblock_function_t *)-1)
typedef void rb_unblock_function_t(void *);
static void *
rb_thread_call_without_gvl(
   void *(*func)(void *), void *data1,
  RB_MYSQL_UNUSED rb_unblock_function_t *ubf,
  RB_MYSQL_UNUSED void *data2)
{
  void *rv;

  TRAP_BEG;
  rv = func(data1);
  TRAP_END;

  return rv;
}

#endif /* ! HAVE_RB_THREAD_BLOCKING_REGION */
#endif /* ! HAVE_RB_THREAD_CALL_WITHOUT_GVL */

typedef struct {
  VALUE encoding;
  VALUE active_thread; /* rb_thread_current() or Qnil */
  long server_version;
  int reconnect_enabled;
  unsigned int connect_timeout;
  int active;
  int connected;
  int initialized;
  int refcount;
  int freed;
  MYSQL *client;
} mysql_client_wrapper;

#define REQUIRE_CONNECTED(wrapper) \
  REQUIRE_INITIALIZED(wrapper) \
  if (!wrapper->connected && !wrapper->reconnect_enabled) { \
    rb_raise(cMysql2Error, "closed MySQL connection"); \
  }

void rb_mysql_client_set_active_thread(VALUE self);

#define MARK_CONN_INACTIVE(conn) do {\
    GET_CLIENT(conn); \
    wrapper->active_thread = Qnil; \
  } while(0)

#define GET_CLIENT(self) \
  mysql_client_wrapper *wrapper; \
  Data_Get_Struct(self, mysql_client_wrapper, wrapper);

void init_mysql2_client();
void decr_mysql2_client(mysql_client_wrapper *wrapper);

#endif

#ifndef HAVE_RB_HASH_DUP
VALUE rb_hash_dup(VALUE other);
#endif
