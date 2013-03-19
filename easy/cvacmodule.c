
// should be first include directive:
#include <Python.h>

/** the exception that Cvac raises on errors
 */
static PyObject *CvacError;

// forward declarations - better kept in .h file?
static PyObject* cvac_version( PyObject *self, PyObject *args);
static PyObject* cvac_corpus( PyObject *self, PyObject *args);

/** Python module method table
 */
static PyMethodDef CvacMethods[] = {
  {"version", cvac_version, METH_VARARGS,
   "Return cvac version string."},
  {"corpus", cvac_corpus, METH_VARARGS,
   "Obtain a particular corpus."},
  {NULL, NULL, 0, NULL}        /* Sentinel */
};

/** Module initialization - must be called initcvac
 *  and is declared as "extern C"
 */
PyMODINIT_FUNC
initcvac(void)
{
  PyObject *m;
  m = Py_InitModule( "cvac", CvacMethods );
  if (m==NULL) return;

  CvacError = PyErr_NewException( "cvac.error", NULL, NULL );
  Py_INCREF( CvacError );
  PyModule_AddObject( m, "error", CvacError );
}

static PyObject *
cvac_version( PyObject *self, PyObject *args)
{
  return Py_BuildValue( "s", "0.1.0" );
}

static PyObject *
cvac_corpus( PyObject *self, PyObject *args)
{
  const char *name;
  if (!PyArg_ParseTuple(args, "s", &name))
    {
      return NULL;
    }
  // find out if anything is known about corpus "name"
  return Py_BuildValue( "ss", name, ": known" );

  // nothing known:
  PyErr_SetString( CvacError, name );

  return Py_BuildValue( "0.1" );
}

