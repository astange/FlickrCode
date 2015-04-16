#include <stdio.h>

unsigned long long put(char * filepath);
int get(unsigned long long photoID);

int main(int argc, char *argv[])
{
    int error = get(atoll(argv[1]));
    printf("%d\n", error);
}

unsigned long long put(char * filepath)
{

}
int get(unsigned long long photoID)
{
    char buf[1000];
    sprintf(buf, "python Back-end.py get %llu\n", photoID);
    const char * mode = "r\0";
    FILE* f = popen(buf,mode);

    if(f != NULL)
    {
        while (fscanf(f, "%s", buf) != EOF) {
            printf("%s\n", buf);
        }
        pclose(f);
    }
    else
    {
        return -1;
    }

    return 0;
}

/*#include <python2.7/Python.h>

static unsigned long long photoID;
static PyObject *

spam_echo(PyObject *self, PyObject *args) {
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    photoID = atoi(command);
    //sts = printf("%s\n", command);
    return Py_BuildValue("i", 0);
}

static PyMethodDef SpamMethods[] = {
    {"echo", spam_echo, METH_VARARGS, "Prints passed argument"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initspam(void) {
    (void) Py_InitModule("spam", SpamMethods);
}

unsigned long long put(char * filepath);
int get(unsigned long long photoID);

int main(int argc, char *argv[])
{
    Py_SetProgramName(argv[0]);
    Py_Initialize();
    get(atoll(argv[1]));
    Py_Finalize();
}

int get(unsigned long long photoID)
{
	FILE*        backEnd;

    PyObject *args;
    PyObject *arg1;
    PyObject *result;
    PyObject *moduleName;
    PyObject *module;
    PyObject *func;

    initspam();

    // Open and execute the Python file
    backEnd = fopen("Back-end.py", "r");
    PyRun_SimpleFileEx(backEnd, "Back-end.py",1);

    moduleName = PyString_FromString("__main__");
    module = PyImport_Import(moduleName);
    Py_DECREF(moduleName);
    if (!module) {
        return 1;
    }

    func = PyObject_GetAttrString(module, "get");
    Py_DECREF(module);
    if (!func || !PyCallable_Check(func)) {
        return 1;
    }

    args = PyTuple_New(1);

    arg1 = Py_BuildValue("K", photoID);

    PyTuple_SetItem(args, 0, arg1);

    result = PyObject_CallObject(func, args);

    Py_DECREF(args);
    Py_DECREF(arg1);
    Py_DECREF(func);


    fclose(backEnd);

	return 1;
}

unsigned long long put(char * filepath)
{
	FILE*        backEnd;

    PyObject *args;
    PyObject *arg1;
    PyObject *result;
    PyObject *moduleName;
    PyObject *module;
    PyObject *func;

    initspam();

    // Open and execute the Python file
    backEnd = fopen("Back-end.py", "r");
    PyRun_SimpleFileEx(backEnd, "Back-end.py",1);

    moduleName = PyString_FromString("__main__");
    module = PyImport_Import(moduleName);
    Py_DECREF(moduleName);
    if (!module) {
        return 1;
    }

    func = PyObject_GetAttrString(module, "put");
    Py_DECREF(module);
    if (!func || !PyCallable_Check(func)) {
        return 1;
    }

    args = PyTuple_New(1);

    arg1 = Py_BuildValue("s", filepath);

    PyTuple_SetItem(args, 0, arg1);

    result = PyObject_CallObject(func, args);

    Py_DECREF(args);
    Py_DECREF(arg1);
    Py_DECREF(func);


    fclose(backEnd);

	return photoID;
}*/