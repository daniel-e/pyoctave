#include <octave/oct.h>
#include <octave/load-path.h>
#include <octave/dNDArray.h>
#include <Python.h>
#include <stdexcept>

// TODO: support for numpy
// #define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
// #include <numpy/ndarraytypes.h>
// PyArray_NDIM(p_ret)

// some useful documents:
// https://docs.python.org/2/c-api/
// https://docs.python.org/2/c-api/float.html
// https://docs.python.org/2/c-api/type.html
// https://docs.python.org/2/c-api/list.html
// https://docs.python.org/2/extending/embedding.html
// https://docs.python.org/2/c-api/
//
// numpy:
// http://docs.scipy.org/doc/numpy/user/basics.creation.html
// http://docs.scipy.org/doc/numpy/reference/c-api.array.html
//
// octave:
// http://octave.sourceforge.net/doxygen/html/db/d88/classload__path.html
//

const char* help_msg = 
	"runpy is an Octave package to run Python scripts.\n"
	"Usage: \n"
	" -- runpy (module_name, f)\n"
	" -- runpy (dbg, module_name, f)\n"
	" -- runpy (module_name, f, arg1, ...)\n"
	" -- runpy (dbg, module_name, f, arg1, ...)\n"
	"\n"
	"  runpy is looking  for the module with the name  'module_name'  in\n"
	"  the function  search path.  If the module  is found the  function\n"
	"  with the name f is executed which can be called with an arbitrary\n"
	"  number of arguments.\n"
	"\n"
	"  In Octave a path  can be  added to the function  search path with\n"
	"  the function  addpath() or it can be specified with the parameter\n"
	"  -p when Octave is started.\n"
	"\n"
	"  NOTE:   Currently the  type of all input  arguments is limited to\n"
	"  string and the type of the return value is limited to a vector of\n"
	"  doubles.\n"
	"\n"
	"  module_name   Name of the module which must exist in the function\n"
	"                search path.\n"
	"\n"
	"  f             Name of the function of the module.\n"
	"\n"
	"  dbg           If true debug output is generated on errors.\n";

void print_usage() {
	octave_stdout << help_msg << std::endl;
}

void update_sys_path() 
{
	PyObject *sysPath = PySys_GetObject((char*) "path");

	auto paths = load_path::dir_list();
	for (auto i = paths.begin(); i != paths.end(); ++i) {
		PyObject *p = PyString_FromString(i->c_str());
		PyList_Append(sysPath, p);
		Py_DECREF(p);
	}
}

class Executer
{
private:
	void debug()
	{ if (_debug) PyErr_Print(); }

	void check(bool b, const std::string& error_msg)
	{ 
		if (b) return;
		debug();
		throw std::runtime_error(error_msg); 
	}

	void get_module_name(const std::string& module_name)
	{
		_p_module_name = PyString_FromString(module_name.c_str());
		check(_p_module_name != NULL, "Could not get module name.");
	}

	void import_module()
	{
		_p_module = PyImport_Import(_p_module_name);
		check(_p_module != NULL, "Could not load module.");
	}

	void get_function(const std::string& func_name)
	{
		_p_func = PyObject_GetAttrString(_p_module, func_name.c_str());
		check(_p_func, "Could not load function.");
		check(PyCallable_Check(_p_func), "Python function is not callable.");
	}

	template<typename Args>
	void parse_args(Args args, int pos)
	{
		_p_args = PyTuple_New(args.length() - pos);
		check(_p_args != NULL, "Could not create argument list.");

		for (int s = pos; pos < args.length(); ++pos) {
			add_arg(_p_args, pos - s, args(pos));
		}
	}

	template<typename Args>
	void check_arguments(Args args, int n)
	{
		if (args.length() >= n) return;
		throw std::runtime_error("Invalid number of arguments. Run 'help runpy' to get help.");
	}
			
	template<typename T>
	void add_arg(PyObject* pArgs, int pos, T value)
	{
		PyObject *pVal = NULL;

		if (value.is_string()) {
			pVal = PyString_FromString(value.string_value().c_str());
		} else {
			throw std::runtime_error("Unsupported argument type.");
		}

		check(pVal != NULL, "Could not convert argument");
		PyTuple_SetItem(pArgs, pos, pVal);
	}

public:
	template<typename Args>
	Executer(Args args)
	: _debug(false),
	  _p_module(NULL), _p_func(NULL), _p_module_name(NULL), _p_args(NULL)
	{
		check_arguments(args, 2);
		int arg_pos = 0;
		if (args(0).is_bool_scalar()) {
			_debug = args(0).bool_value();
			arg_pos++;
		}
		check_arguments(args, 3);
		get_module_name(args(arg_pos++).string_value());
		import_module();
		get_function(args(arg_pos++).string_value());
		parse_args(args, arg_pos);
	}

	~Executer()
	{
		if (_p_module) Py_DECREF(_p_module);
		if (_p_func) Py_XDECREF(_p_func);
		if (_p_module_name) Py_DECREF(_p_module_name);
		if (_p_args) Py_DECREF(_p_args);
		// TODO decrement ref counter of values of _p_args
	}

	std::vector<double> return_values(PyObject* p_ret)
	{
		int n = PyList_Size(p_ret);
		bool err = false;

		std::vector<double> r;
		for (int i = 0; i < n; ++i) {
			PyObject *val = PyList_GetItem(p_ret, i);
			if (PyFloat_Check(val)) {
				r.push_back(PyFloat_AsDouble(val));
			} else if (PyInt_Check(val)) {
				r.push_back(PyInt_AsLong(val));
			} else err = true;
			Py_DECREF(val);
		}
		if (!err) return r;
		throw std::runtime_error("Found invalid type in list object.");
	}

	template<typename T>
	NDArray octave_vector(const T& values)
	{
		dim_vector dv(1, 1);
		dv(0) = values.size();
		NDArray arr(dv);
		for (int i = 0; i < values.size(); ++i) {
			arr(i) = values[i];
		}
		return arr;
	}

	NDArray run()
	{
		PyObject *p_ret = PyObject_CallObject(_p_func, _p_args);
		check(p_ret != NULL, "Call to Python function failed.");

		if (!PyList_Check(p_ret)) {
			Py_DECREF(p_ret);
			check(false, "Return value is not a list object.");
		}

		NDArray arr = octave_vector(return_values(p_ret));
		Py_DECREF(p_ret);
		return arr;
	}

private:
	bool      _debug;
	PyObject *_p_module;
	PyObject *_p_func;
	PyObject *_p_module_name;
	PyObject *_p_args;
};

DEFUN_DLD (runpy, args, nargout, help_msg)
{
	Py_Initialize();
	update_sys_path();
	NDArray ret;
	try {
		Executer e(args);
		ret = e.run();
	} catch (const std::exception& e) {
		octave_stdout << e.what() << std::endl;
	}
	Py_Finalize();
	return octave_value(ret);
}
