#include "gcc-plugin.h"
#include "basic-block.h"
#include "memmodel.h"
#include "rtl.h"
#include "emit-rtl.h"
#include "tree.h"
#include "tree-pass.h"
#include "output.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include "options.h"

// Assert that this plugin is GPL compatibly licensed.
int plugin_is_GPL_compatible;

// A data structure for holding the file pointer to
// the file where we will write out information about
// whether the function in a compilation are leaf or not.
struct leaf_logger_options {
    FILE *output_file;
};
// Make a global one so that we can use it from the
// different places without worrying about it being
// deallocated.
static struct leaf_logger_options *ll_options;

// Define our logger as a subclass of an rtl optimization pass.
class leaf_logger_opt_pass : public rtl_opt_pass {

public:
  leaf_logger_opt_pass(const pass_data &data) : rtl_opt_pass(data, nullptr) {}

  // This function is called when more than one
  // of these are needed. Still fuzzy on the details.
  // More later.
  leaf_logger_opt_pass *clone() {
      return this;
  }

  // The code that gets executed by the pass
  unsigned int execute(function *func) {
      FILE *output_file = ll_options->output_file;
      fprintf(output_file,
	      "%s, ", IDENTIFIER_POINTER(DECL_NAME(func->decl)));
      const bool is_leaf = crtl->is_leaf;
      if (is_leaf)
	{
	  fprintf(output_file, "leaf");
	}
      else
	{
	  fprintf(output_file, "not leaf");
	}
      fprintf(output_file, "\n");
      return 0;
  }
};

int ll_initialize_output(char *filename)
{
  if (ll_options == NULL)
    {
      fprintf(stderr,
	      "leaf_logger_diagnostic: Options structure not allocated.\n");
      return 1;
    }

  if (!(ll_options->output_file = fopen(filename, "w+")))
    {
      fprintf(stderr,
	      "leaf_logger_diagnostic: Could not open output file: %s\n", 
	      xstrerror(errno));
      return 1;
    }
  return 0;
}

void ll_diagnostic_stop(void *data, void *user)
{
  if (ll_options != NULL && ll_options->output_file != NULL)
    fclose(ll_options->output_file);
  return;
}

int usage(void)
{
  fprintf(stderr, "usage: -fplugin=<path to leaf_logger.so>/leaf_logger.so\n");
  fprintf(stderr, "      [-fplugin-arg-leaf_logger-output=<output filename>]\n");
  return 1;
}

int plugin_init(struct plugin_name_args *plugin_info,
		struct plugin_gcc_version *version)
{
  struct register_pass_info ll_pass_info;
  char *filename = NULL;
  struct pass_data ll_pass_data;

  if (plugin_info->argc == 0)
    {
      // Generate an output name for the user.
      unsigned int filename_alloc_len=(strlen(main_input_filename)+
				       1+ /* for the . */
				       6); /* for the leafs */
      filename_alloc_len *= sizeof(char);

      filename = (char*)xmalloc(filename_alloc_len);
      memset(filename, 0, filename_alloc_len);
      strcpy(filename, main_input_filename);
      strcat(filename, ".leafs");

      for (int i = 0; i<strlen(filename); i++)
	if (filename[i] == '/')
	  filename[i] = '_';
    }
  else if (plugin_info->argc == 1)
    {
      // If it passes the sniff test, let the user set an output name.
      if (strcmp(plugin_info->argv[0].key, "output"))
	return usage();
      filename = plugin_info->argv[0].value;
    }
  else
    // Otherwise, we have to tell them how to use the plugin!
    return usage();

  ll_options = (struct leaf_logger_options*)xmalloc(sizeof(struct
							   leaf_logger_options));

  memset(ll_options, 0, sizeof(struct leaf_logger_options));
  memset(&ll_pass_info, 0, sizeof(struct register_pass_info));
  memset(&ll_pass_data, 0, sizeof(struct pass_data));

  if (ll_initialize_output(filename))
    return 1;

  ll_pass_data.type = RTL_PASS;
  ll_pass_data.name = "leaf_logger_plugin";

  leaf_logger_opt_pass *ll_pass = new leaf_logger_opt_pass(ll_pass_data);

  ll_pass_info.reference_pass_name = "final";
  ll_pass_info.pos_op = PASS_POS_INSERT_AFTER;
  ll_pass_info.pass = ll_pass;

  register_callback(plugin_info->base_name,
		    PLUGIN_PASS_MANAGER_SETUP,
		    NULL,
		    &ll_pass_info);

  register_callback(plugin_info->base_name,
		    PLUGIN_FINISH,
		    ll_diagnostic_stop,
		    NULL);
  return 0;
}
