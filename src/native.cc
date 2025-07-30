//
// Created by Yue Xue  on 12/23/24.
//

#include "native.hpp"
#include "object.hpp"
#include "liblox_core.h"
#include "liblox_iter.h"
#include "liblox_data_structure.h"
#include "vm.hpp"
#include "stdlib.h"
#include "string.h"
#include "time.h"

#define FNV_PRIME 16777619
#define FNV_OFFSET_BASIS 2166136261

Class *array_class;
Class *string_class;
Class *int_class;
Class *float_class;
Class *bool_class;
Class *native_class;
Class *class_class;
Class *function_class;
Class *closure_class;
Class *map_class;
Class *method_class;
Class *nil_class;
Class *module_class;
Class *native_object_class;
Class *native_method_class;
Class *file_class;

Class *Error;
Class *TypeError;
Class *IndexError;
Class *ArgError;
Class *NameError;
Class *PropertyError;
Class *ValueError;
Class *FatalError;
Class *CompileError;
Class *IOError;

String *INIT = NULL;
String *LENGTH = NULL;
String *ITERATOR = NULL;
String *HAS_NEXT = NULL;
String *NEXT = NULL;
String *EQUAL = NULL;
String *HASH = NULL;
String *MESSAGE = NULL;
String *POSITION = NULL;
String *FD = NULL;

/**
 * @return true: one等于two，或者one是two的子类
 */
bool is_subclass(Class *one, Class *two) {
    if (one == two) {
        return true;
    }
    if (one->super_class == NULL) {
        return false;
    }
    return is_subclass(one->super_class, two);
}

static Value native_class_method_subclass_of(int count, Value *values) {
    (void ) count;
    Value v0 = values[-1];
    Value v1 = values[0];
    assert_ref_type(v0, OBJ_CLASS, "class");
    assert_ref_type(v1, OBJ_CLASS, "class");
    bool res = is_subclass(as_class(v0), as_class(v1));
    return bool_value(res);
}

static Value native_is_object(int count, Value *value) {
    (void ) count;
    Value v = *value;
    return bool_value(is_ref_of(v, OBJ_INSTANCE));
}

/**
 *
 * @param count class的数量。
 * @param values values[0] 为判断的目标，其后的所有values都是class
 * @return 如果values[0]属于后面任何一个class的值，返回true，否则false
 */
bool multi_value_of(int count, Value *values) {
    Value v = values[0];
    Class *class = value_class(v);
    bool result = false;
    for (int i = 1; i <= count; ++i) {
        Value arg_class = values[i];
        assert_ref_type(arg_class, OBJ_CLASS, "class");
        if (is_ref_of(v, OBJ_INSTANCE)) {
            result = is_subclass(class, as_class(arg_class));
        } else {
            result = class == as_class(arg_class);
        }
        if (result) {
            return true;
        }
    }
    return false;
}

static Value native_value_of(int count, Value *values) {
    (void ) count;
    bool result = multi_value_of(1, values);
    return bool_value(result);
}

Value native_backtrace(int count, Value *value) {
    (void ) count;
    (void ) value;
    int total_len = 0;
    char **css = malloc(sizeof(char *) * vm.frame_count);
    int *lens = malloc(sizeof(int) * vm.frame_count);

    for (int i = vm.frame_count - 1; i >= 0; i--) {
        CallFrame *frame = vm.frames + i;
        LoxFunction *function = frame->closure->function;
        size_t index = frame->PC - frame->closure->function->chunk.code - 1;
        int line = function->chunk.lines[index];
        char *name = value_to_chars(ref_value((Object *) frame->closure), NULL);
        char *module_name = value_to_chars(ref_value((Object *)frame->closure->module_of_define), NULL);
        lens[i] = asprintf(css + i, "at [line %d] in %s of %s\n", line, name, module_name);
        free(name);
        free(module_name);
        total_len += lens[i];
    }
    char *result = malloc(total_len + 1);
    char *curr = result;
    for (int i = vm.frame_count - 1; i >= 0; i --) {
        memcpy(curr, css[i], lens[i]);
        curr += lens[i];
        free(css[i]);
    }
    result[total_len] = '\0';
    free(css);
    free(lens);
    String *str = string_copy(result, total_len);
    return ref_value((Object *) str);
}

/**
 * 生成一个新的指定类型的error对象，然后将其置入栈顶。
 */
void new_error(ErrorType type, const char *message) {
    Class *error_class;
    switch (type) {
        case Error_Error:
            error_class = Error;
            break;
        case Error_IndexError:
            error_class = IndexError;
            break;
        case Error_NameError:
            error_class = NameError;
            break;
        case Error_PropertyError:
            error_class = PropertyError;
            break;
        case Error_TypeError:
            error_class = TypeError;
            break;
        case Error_ValueError:
            error_class = ValueError;
            break;
        case Error_ArgError:
            error_class = ArgError;
            break;
        case Error_FatalError:
            error_class = FatalError;
            break;
        case Error_CompileError:
            error_class = CompileError;
            break;
        case Error_IOError:
            error_class = IOError;
            break;
    }
    Instance *error_instance = new_instance(error_class);
    stack_push(ref_value((Object *)error_instance));
    String *message_str = auto_length_string_copy(message);
    stack_push(ref_value((Object *) message_str));
    table_set(&error_instance->fields, MESSAGE, ref_value((Object*) message_str));
    stack_pop();
}

static void define_native(const char *name, NativeImplementation impl, int arity) {
//    int len = (int) strlen(name);

    String *native_name = auto_length_string_copy(name);
    stack_push(ref_value_cast(native_name));

    NativeFunction *fun = new_native(impl, native_name, arity);
    Value fun_value = ref_value_cast(fun);
    stack_push(fun_value);

    table_add_new(&vm.builtin, native_name, fun_value, true, false);

    stack_pop();
    stack_pop();
}

static void add_native_method(Class *class, const char *name, NativeImplementation impl, int arity) {

    String *native_name = auto_length_string_copy(name);
    stack_push(ref_value_cast(native_name));

    NativeFunction *fun = new_native(impl, native_name, arity);
    Value fun_value = ref_value_cast(fun);
    stack_push(fun_value);

    table_add_new(&class->methods, native_name, fun_value, true, false);

    stack_pop();
    stack_pop();
}

static void add_builtin_field(const char *name, Value value) {
    table_add_new(&vm.builtin, auto_length_string_copy(name), value, true, true);
}

static void add_native_class_static_function(Class *class, const char *name, NativeImplementation impl, int arity) {
    String *native_name = auto_length_string_copy(name);
    stack_push(ref_value_cast(native_name));

    NativeFunction *fun = new_native(impl, native_name, arity);
    Value fun_value = ref_value_cast(fun);
    stack_push(fun_value);

    table_add_new(&class->static_fields, native_name, fun_value, true, false);

    stack_pop();
    stack_pop();
}

static Value native_map_method_delete(int count, Value *values) {
    (void ) count;
    assert_ref_type(values[-1], OBJ_MAP, "Map"); // [map, key]
    map_delete(); // [map, key, value]
    return stack_pop();
}

/**
 * 类似于memcpy
 * @param values [src, dest, src_start_index, dest_start_index, len_to_copy]。
 */
static Value native_array_copy(int count, Value *values) {
    (void ) count;
    assert_ref_type(values[0], OBJ_ARRAY, "Array");
    assert_ref_type(values[1], OBJ_ARRAY, "Array");
    assert_value_type(values[2], VAL_INT, "Int");
    assert_value_type(values[3], VAL_INT, "Int");
    assert_value_type(values[4], VAL_INT, "Int");
    Array *src = as_array(values[0]);
    Array *dest = as_array(values[1]);
    int src_start_index = as_int(values[2]);
    int dest_start_index = as_int(values[3]);
    int len_to_copy = as_int(values[4]);
    if (len_to_copy == 0) {
        return nil_value();
    }

    if (len_to_copy < 0 || src_start_index < 0 || dest_start_index < 0 || src_start_index + len_to_copy > src->length || dest_start_index + len_to_copy > dest->length) {
        throw_new_runtime_error(Error_ValueError, "ValueError: the range to copy is out of bound");
    }
    if (src == dest) {
        memmove(dest->values + dest_start_index, src->values + src_start_index, len_to_copy * sizeof(Value));
    } else {
        memcpy(dest->values + dest_start_index, src->values + src_start_index, len_to_copy * sizeof(Value));
    }
    return nil_value();
}


static Value native_type(int count, Value *value) {
    (void ) count;
    return ref_value((Object *)value_class(*value));
}

static inline int max(int a, int b) {
    return a > b ? a : b;
}

static inline bool is_placeholder(const char *str) {
    return str[0] == '{' && str[1] == '}';
}

static Value native_string_method_format(int count, Value *values) {
    const char *format = as_string(values[-1])->chars;
    static int capacity = 0;
    static char *buf = NULL;
    int pre = 0;
    int curr = 0;
    int buf_len = 0;
    int curr_v = 0;

    while (true) {
        while (format[curr] != '\0' && !is_placeholder(format + curr)) {
            curr++;
        }

        int new_len = curr - pre;
        // buf overflow checking
        if (buf_len + new_len > capacity) {
            capacity = max(2 * capacity, buf_len + new_len);
            buf = realloc(buf, capacity); // NOLINT
        }

        memcpy(buf + buf_len, format + pre, new_len);
        buf_len += new_len;
        pre = curr;

        if (format[curr] == '\0') {
            if (curr_v != count) {
                free(buf);
                throw_new_runtime_error(Error_ArgError, "ArgError: more arguments than placeholders");
            }
            break;
        }

        if (is_placeholder(format + curr)) {
            if (curr_v == count) {
                free(buf);
                throw_new_runtime_error(Error_ArgError, "ArgError: more placeholders than arguments");
            }

            Value v = values[curr_v];
            int v_chars_len;
            char *v_chars = value_to_chars(v, &v_chars_len);

            if (v_chars_len + buf_len > capacity) {
                capacity = max(2 * capacity, v_chars_len + buf_len);
                buf = realloc(buf, capacity); // NOLINT
            }
            memcpy(buf + buf_len, v_chars, v_chars_len);
            buf_len += v_chars_len;
            curr += 2;
            pre = curr;
            curr_v++;
            free(v_chars);
        }
    }

    String *string = string_copy(buf, buf_len);
    return ref_value((Object *) string);
}

static Value native_clock(int count, Value *value) {
    (void) count;
    (void) value;
    return float_value((double) clock() / CLOCKS_PER_SEC);
}

static Value native_int(int count, Value *value) {
    (void) count;
    switch (value->type) {
        case VAL_INT:
            return *value;
        case VAL_FLOAT:
            return int_value((int) as_float(*value));
        case VAL_BOOL:
            return int_value((int) as_bool(*value));
        default:
            if (is_ref_of(*value, OBJ_STRING)) {
                String *str = as_string(*value);
                char *end;
                int result = strtol(str->chars, &end, 10); // NOLINT
                if (end == str->chars) {
                    throw_new_runtime_error(Error_ValueError, "ValueError: not a valid int: %s", str->chars);
                }
                return int_value(result);
            }
            throw_new_runtime_error(Error_ValueError, "ValueError: not a valid input");
            return nil_value();
    }
}

static Value native_float(int count, Value *value) {
    (void) count;
    Value v = *value;
    switch (v.type) {
        case VAL_INT:
            return float_value((double) as_int(v));
        case VAL_FLOAT:
            return v;
        case VAL_BOOL:
            return float_value((double) as_bool(v));
        default:
            if (is_ref_of(v, OBJ_STRING)) {
                String *str = as_string(v);
                char *end;
                double result = strtod(str->chars, &end);
                if (end == str->chars) {
                    throw_new_runtime_error(Error_ValueError, "ValueError: not a valid float: %s", str->chars);
                }
                return float_value(result);
            }
            throw_new_runtime_error(Error_ValueError, "ValueError: not a valid input");
            return nil_value();
    }
}

static Value native_help(int count, Value *value) {
    (void) count;
    (void) value;
    printf("You are in the REPL mode because you run clox without providing a path to a script.\n");
    printf("You can also do `clox path/to/script` to run a lox script.\n");
    printf("Or do `clox -h` to see more options\n");
    printf("In this REPL mode, expression results will be printed out automatically in gray color. \n");
    printf("You may also omit the last semicolon for a statement.\n");
    printf("Use exit(), ctrl+C or ctrl+D to quit.\n");
    return nil_value();
}

static Value native_exit(int count, Value *value) {
    (void ) count;
    (void ) value;
    longjmp(error_buf, INTERPRET_REPL_EXIT);
    return nil_value();
}

static Value native_rand(int count, Value *value) {
    (void) count;
    Value a = value[0];
    Value b = value[1];
    assert_value_type(a, VAL_INT, "int");
    assert_value_type(b, VAL_INT, "int");
    return int_value(as_int(a) + rand() % as_int(b)); // NOLINT(*-msc50-cpp)
}

static Value native_string_combine(int count, Value *values) {
    int total_len = 0;
    char **css = malloc(sizeof(char *) * count);
    int *lens = malloc(sizeof(int ) * count);
    for (int i = 0; i < count; ++i) {
        css[i] = value_to_chars(values[i], lens + i);
        total_len += lens[i];
    }
    char *result= malloc(total_len + 1);
    char *curr = result;
    for (int i = 0; i < count; ++i) {
        memcpy(curr, css[i], lens[i]);
        curr += lens[i];
        free(css[i]);
    }
    result[total_len] = '\0';
    free(lens);
    free(css);
    String *str = string_allocate(result, total_len);
    return ref_value((Object *) str);
}

static Value native_string_combine_array(int count, Value *value) {
    (void ) count;
    Value v = *value;
    assert_ref_type(v, OBJ_ARRAY, "array");
    Array *array = as_array(v);
    return native_string_combine(array->length, array->values);
}

static Value native_string_join(int count, Value *values) {
    // 0: delimiter, 1: prefix, 2: suffix, 3: array
    (void ) count;
    Value v0 = values[0];
    Value v1 = values[1];
    Value v2 = values[2];
    Value v3 = values[3];
    assert_ref_type(v0, OBJ_STRING, "string");
    assert_ref_type(v1, OBJ_STRING, "string");
    assert_ref_type(v2, OBJ_STRING, "string");
    assert_ref_type(v3, OBJ_ARRAY, "array");
    String *delimiter = as_string(v0);
    String *prefix = as_string(v1);
    String *suffix = as_string(v2);
    Array *array = as_array(v3);
    int array_len = array->length;

    int total_len = prefix->length + suffix->length + delimiter->length * (array_len - 1);

    for (int i = 0; i < array_len; ++i) {
        Value v = array->values[i];
        assert_ref_type(v, OBJ_STRING, "string");
        total_len += as_string(v)->length;
    }
    char *result= malloc(total_len + 1);
    char *curr = result;
    memcpy(curr, prefix->chars, prefix->length);
    curr += prefix->length;
    for (int i = 0; i < array_len; ++i) {
        String *s = as_string(array->values[i]);
        memcpy(curr, s->chars, s->length);
        curr += s->length;
        if (i != array_len - 1) {
            memcpy(curr, delimiter->chars, delimiter->length);
            curr += delimiter->length;
        }
    }
    memcpy(curr, suffix->chars, suffix->length);
    result[total_len] = '\0';
    String *str = string_allocate(result, total_len);
    return ref_value((Object *) str);
}

/**
 * @param values [old_str, start_index, end_index, str_to_insert]
 * @return new string
 */
static Value native_string_method_replace(int count, Value *values) {
    (void ) count;
    assert_value_type(values[0], VAL_INT, "Int");
    assert_value_type(values[1], VAL_INT, "Int");
    assert_ref_type(values[2], OBJ_STRING, "String");

    // "animal", 2, 4, "moria" -> an[moria]al

    String *old_str = as_string(values[-1]);
    int start = as_int(values[0]);
    int end = as_int(values[1]);
    String *new_str = as_string(values[2]);
    if (start < 0 || end > old_str->length || start > end) {
        throw_new_runtime_error(Error_ValueError, "the range: [%s, %s] is invalid", start, end);
    }
    int new_len = old_str->length - (end - start) + new_str->length;
    char *cs = malloc(new_len + 1);
    memcpy(cs, old_str->chars, start);
    memcpy(cs + start, new_str->chars, new_str->length);
    memcpy(cs + start + new_str->length, old_str->chars + end, old_str->length - end);
    cs[new_len] = '\0';
    String *result = string_allocate(cs, new_len);
    return ref_value_cast(result);
}

static Value native_string_method_strip(int count, Value *values) {
    (void ) count;
    String *str = as_string(values[-1]);
    char *start = str->chars;
    char *end = str->chars + str->length - 1; // a, b, c, d
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }
    while (end > start && ( *end == ' ' || *end == '\t' || *end == '\n' )) {
        end--;
    }
    int len = end - start + 1;
    String *res = string_copy(start, len);
    return ref_value_cast(res);
}

/**
 * @param value [receiver(str), start, end]
 */
static Value native_string_method_substring(int count, Value *value) {
    (void ) count;
    assert_value_type(value[0], VAL_INT, "Int");
    assert_value_type(value[1], VAL_INT, "Int");
    String *str = as_string(value[-1]);
    int start = as_int(value[0]);
    int end = as_int(value[1]);
    if (start < 0 || end > str->length) {
        throw_new_runtime_error(Error_ValueError, "the range: [%d, %d] is invalid", start, end);
    }
    String *result = string_copy(str->chars + start, (end - start));
    return ref_value_cast(result);
}

static Value native_value_join(int count, Value *values) {
   // 0: delimiter, 1: prefix, 2: suffix, 3: array
    (void ) count;
    Value v0 = values[0];
    Value v1 = values[1];
    Value v2 = values[2];
    Value v3 = values[3];
    assert_ref_type(v0, OBJ_STRING, "string");
    assert_ref_type(v1, OBJ_STRING, "string");
    assert_ref_type(v2, OBJ_STRING, "string");
    assert_ref_type(v3, OBJ_ARRAY, "array");
    String *delimiter = as_string(v0);
    String *prefix = as_string(v1);
    String *suffix = as_string(v2);
    Array *array = as_array(v3);
    int array_len = array->length;

    int total_len = prefix->length + suffix->length + delimiter->length * (array_len - 1);
    char **css = malloc(sizeof(char *) * array_len);
    int *lens = malloc(sizeof(int ) * array_len);

    for (int i = 0; i < array_len; ++i) {
        css[i] = value_to_chars(array->values[i], lens + i);
        total_len += lens[i];
    }
    char *result= malloc(total_len + 1);
    char *curr = result;
    memcpy(curr, prefix->chars, prefix->length);
    curr += prefix->length;
    for (int i = 0; i < array_len; ++i) {
        memcpy(curr, css[i], lens[i]);
        curr += lens[i];
        free(css[i]);
        if (i != array_len - 1) {
            memcpy(curr, delimiter->chars, delimiter->length);
            curr += delimiter->length;
        }
    }
    memcpy(curr, suffix->chars, suffix->length);
    result[total_len] = '\0';
    free(lens);
    free(css);
    String *str = string_allocate(result, total_len);
    return ref_value((Object *) str);
}

static Value native_string_method_char_at(int count, Value *value) {
    (void )count;
    assert_ref_type(value[-1], OBJ_STRING, "string");
    assert_value_type(value[0], VAL_INT, "int");
    String *string = as_string(value[-1]);
    int index = as_int(value[0]);
    if (index < 0 || index >= string->length) {
        throw_new_runtime_error(Error_IndexError, "IndexError: index %d is out of bound: [%d, %d]", index, 0, string->length - 1);
    }
    String *c = string_copy(string->chars+index, 1);
    return ref_value((Object *) c);
}

static Value native_range(int count, Value *value) {
    // curr, limit, step
    (void ) count;
    NativeObject *native_object = new_native_object(NativeRangeIter, 3);
    native_object->values[2] = value[2]; // step
    native_object->values[0] = int_value(as_int(value[0]) - as_int(value[2])); // curr
    native_object->values[1] = int_value(as_int(value[1]) - as_int(value[2])); // limit
    return ref_value((Object *) native_object);
}

static Value native_array_iter(int count, Value *value) {
    (void ) count;
    // curr, array, limit
    assert_ref_type(*value, OBJ_ARRAY, "array");
    NativeObject *nativeObject = new_native_object(NativeArrayIter, 3);
    nativeObject->values[0] = int_value(0);
    nativeObject->values[1] = *value;
    nativeObject->values[2] = value[1];
    return ref_value((Object *) nativeObject);
}

uint32_t value_hash(Value given) {
    uint32_t hash = FNV_OFFSET_BASIS;
    switch(given.type) {
        case VAL_INT: { // int
            int value = as_int(given);
            const unsigned char* bytes = (const unsigned char*)&value;
            for (size_t i = 0; i < sizeof(int); i++) {
                hash ^= bytes[i];
                hash *= FNV_PRIME;
            }
            return hash;
        }
        case VAL_FLOAT: { // double
            double value = as_float(given);
            const unsigned char* bytes = (const unsigned char*)&value;
            for (size_t i = 0; i < sizeof(double); i++) {
                hash ^= bytes[i];
                hash *= FNV_PRIME;
            }
            return hash;
        }
        case VAL_BOOL: { // bool
            bool value = as_bool(given);
            hash ^= value ? 1 : 0;
            hash *= FNV_PRIME;
            return hash;
        }
        case VAL_NIL: {
            return 0;
        }
        case VAL_REF: { // pointer
            if (is_ref_of(given, OBJ_STRING)) {
                String *string = as_string(given);
                return string->hash;
            } else {
                uintptr_t ptr = (uintptr_t) as_ref(given);
                const unsigned char* bytes = (const unsigned char*)&ptr;
                for (size_t i = 0; i < sizeof(void*); i++) {
                    hash ^= bytes[i];
                    hash *= FNV_PRIME;
                }
                return hash;
            }
        }
        default:
            IMPLEMENTATION_ERROR("bad");
            return 0;
    }
}

static Value native_method_general_hash(int count, Value *given) {
    (void ) count;
    Value v = given[-1];
    int hash = (int) value_hash(v);
    return int_value(hash);
}

Value native_method_value_equal(int count, Value *values) {
    (void ) count;
    Value a = values[-1];
    Value b = values[0];
    if (a.type != b.type) {
        return bool_value(false);
    }
    bool result;
    switch (a.type) {
        case VAL_BOOL:
            result = as_bool(a) == as_bool(b);
            break;
        case VAL_INT:
            result = as_int(a) == as_int(b);
            break;
        case VAL_FLOAT:
            result =  as_float(a) == as_float(b);
            break;
        case VAL_NIL:
        case VAL_ABSENCE:
            result = true;
            break;
        case VAL_REF: {
            result = as_ref(a) == as_ref(b);
            break;
        }
    }
    return bool_value(result);
}

Value native_map_iter(int count, Value *value) {
    (void ) count;
    assert_ref_type(*value, OBJ_MAP, "Map");
    NativeObject *nativeObject = new_native_object(NativeMapIter, 2);
    // curr, map
    nativeObject->values[0] = int_value(0);
    nativeObject->values[1] = *value;
    return ref_value((Object*) nativeObject);
}

Value native_open_file(int count, Value *value) {
    (void ) count;
    assert_ref_type(value[0], OBJ_STRING, "String");
    assert_ref_type(value[1], OBJ_STRING, "String");
    assert_value_type(value[2], VAL_INT, "Int");
    String *path = as_string(value[0]);
    String *mode = as_string(value[1]);
    int fd = as_int(value[2]);
    Value res;
    res.type = VAL_ABSENCE;
    switch (fd) {
        case 0:
            res.as.reference = (Object *) stdin;
            break;
        case 1:
            res.as.reference = (Object *) stdout;
            break;
        case 2:
            res.as.reference = (Object *) stderr;
            break;
        default: {
            FILE *file = fopen(path->chars, mode->chars);
            if (file == NULL) {
                throw_new_runtime_error(Error_IOError, "IOError: cannot open the file: %s", path->chars);
            }
            res.as.reference = (Object *) file;
            break;
        }
    }
    return res;
}

FILE *native_file_method_helper_get_file(Value *value) {
    assert_ref_type(value[-1], OBJ_INSTANCE, "File");
    Instance *ins = as_instance(value[-1]);
    Value temp;
    table_get(&ins->fields, FD, &temp);
    FILE *file = (FILE *) temp.as.reference;
    if (file == NULL) {
        throw_new_runtime_error(Error_IOError, "IOError: the file is closed");
    }
    return file;
}

Value native_file_method_read_chars(int count, Value *value) {
    // file, limit
    (void ) count;
    int limit = as_int(value[0]);
    if (limit <= 0) {
        throw_new_runtime_error(Error_ValueError, "ValueError: can only read positive amount of chars, but got: %d", limit);
    }
    FILE *file = native_file_method_helper_get_file(value);
    char *buf = malloc(limit + 1);
    if (fgets(buf, limit + 1, file)) {
        String *str = auto_length_string_copy(buf);
        free(buf);
        return ref_value_cast(str);
    } else if (ferror(file)){
        free(buf);
        throw_new_runtime_error(Error_IOError, "IOError: error when reading from the file");
        return nil_value();
    } else {
        free(buf);
        return nil_value();
    }
}

Value native_file_method_read_line(int count, Value *value) {
    (void ) count;
    FILE *file = native_file_method_helper_get_file(value);
    char *buf = NULL;
    size_t cap = 0;
    int len = getline(&buf, &cap, file);
    if (len != -1) {
        String *line = string_copy(buf, len);
        free(buf);
        return ref_value_cast(line);
    } else if (ferror(file)){
        free(buf);
        throw_new_runtime_error(Error_IOError, "IOError: error when reading from the file");
        return nil_value();
    } else {
        free(buf);
        return nil_value();
    }
}

static Value native_file_method_read_all(int count, Value *value) {
    (void ) count;
    FILE *file = native_file_method_helper_get_file(value);
    if (feof(file)) {
        return nil_value();
    }

    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *buffer = malloc(size + 1);
    assert(buffer != NULL);
    size_t read_size = fread(buffer, 1, size + 1, file); // size + 1 to set eof flag
    if (ferror(file)) {
        free(buffer);
        throw_new_runtime_error(Error_IOError, "IOError: error when reading the file");
        return nil_value();
    }
    buffer[read_size] = '\0';
    String *str = string_allocate(buffer, read_size);
    return ref_value_cast(str);
}

Value native_file_method_write(int count, Value *value) {
    // count: 1 or 2
    assert_ref_type(value[0], OBJ_STRING, "String");
    String *str = as_string(value[0]);
    bool new_line;
    if (count == 1) {
        new_line = false;
    } else if (count == 2) {
        new_line = test_bool(value[1]);
    } else {
        throw_new_runtime_error(Error_ArgError, "ArgError: expect 1 or 2 arguments, but got %d", count);
    }

    FILE *file = native_file_method_helper_get_file(value);
    if (EOF == fputs(str->chars, file)) {
        throw_new_runtime_error(Error_IOError, "IOError: error when writing to the file");
    } else if (new_line){
        fputs("\n", file);
    }
    return nil_value();
}

Value native_file_method_close(int count, Value *value) {
    (void ) count;
    Instance *ins = as_instance(value[-1]);
    Value temp;
    table_get(&ins->fields, FD, &temp);
    FILE *file = (FILE *) temp.as.reference;
    if (file == NULL) {
        throw_new_runtime_error(Error_IOError, "IOError: the file is closed");
    }
    fclose(file);
    temp.as.reference = NULL;
    return nil_value();
}

Value native_file_method_seek(int count, Value *value) {
    int whence;
    if (count == 1) {
        whence = 0;
    } else if (count == 2) {
        assert_value_type(value[1], VAL_INT, "Int");
        whence = as_int(value[1]);
    } else {
        whence = 0;
        throw_new_runtime_error(Error_ArgError, "ArgError: expect 1 or 2 arguments, but got %d", count);
    }
    assert_value_type(value[0], VAL_INT, "Int");
    int offset = as_int(value[0]);

    FILE *file = native_file_method_helper_get_file(value);

    fseek(file, offset, whence);
    return nil_value();
}

Value native_file_method_tell(int count, Value *value) {
    (void ) count;
    FILE *file = native_file_method_helper_get_file(value);
    int position = ftell(file);
    return int_value(position);
}

void init_vm_native() {
    define_native("clock", native_clock, 0);
    define_native("int", native_int, 1);
    define_native("float", native_float, 1);
    define_native("rand", native_rand, 2);
    define_native("type", native_type, 1);
    define_native("native_string_combine_array", native_string_combine_array, 1);
    define_native("native_value_join", native_value_join, 4);
    define_native("native_string_join", native_string_join, 4);
    define_native("native_range", native_range, 3);
    define_native("native_array_iter", native_array_iter, 2);
    define_native("native_map_iter", native_map_iter, 1);
    define_native("backtrace", native_backtrace, 0);
    define_native("value_of", native_value_of, 2);
    define_native("is_object", native_is_object, 1);
    define_native("native_open_file", native_open_file, 3);
}

void additional_repl_init() {
    define_native("help", native_help, 0);
    define_native("exit", native_exit, -1);
}

void init_static_strings() {
    INIT = auto_length_string_copy("init");
    LENGTH = auto_length_string_copy("length");
    ITERATOR = auto_length_string_copy("iterator");
    HAS_NEXT = auto_length_string_copy("has_next");
    NEXT = auto_length_string_copy("next");
    EQUAL = auto_length_string_copy("equal");
    HASH = auto_length_string_copy("hash");
    MESSAGE = auto_length_string_copy("message");
    POSITION = auto_length_string_copy("position");
    FD = auto_length_string_copy("fd");
}

/**
 * 如果 LOAD_LIB 不为false，则加载标准库（执行字节码），将标准库的成员导入builtin命名空间中。
 */
void load_libraries() {
    if (COMPILE_ONLY) {
        return;
    }

    if (load_bytes_into_builtin(liblox_iter, liblox_iter_len, "lib_iter") != INTERPRET_EXECUTE_OK) {
        exit(1);
    }
    if (load_bytes_into_builtin(liblox_core, liblox_core_len, "lib_core") != INTERPRET_EXECUTE_OK) {
        exit(1);
    }
    if (load_bytes_into_builtin(liblox_data_structure, liblox_data_structure_len, "lib_data_structure") != INTERPRET_EXECUTE_OK) {
        exit(1);
    }
    Value class_value;

    table_get(&vm.builtin, auto_length_string_copy("Array"), &class_value);
    array_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("String"), &class_value);
    string_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Int"), &class_value);
    int_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Float"), &class_value);
    float_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Bool"), &class_value);
    bool_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Native"), &class_value);
    native_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Class"), &class_value);
    class_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Function"), &class_value);
    function_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Closure"), &class_value);
    closure_class= as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Method"), &class_value);
    method_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Module"), &class_value);
    module_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Nil"), &class_value);
    nil_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Map"), &class_value);
    map_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("NativeObject"), &class_value);
    native_object_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("NativeMethod"), &class_value);
    native_method_class = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("Error"), &class_value);
    Error = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("TypeError"), &class_value);
    TypeError = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("ArgError"), &class_value);
    ArgError = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("IndexError"), &class_value);
    IndexError = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("NameError"), &class_value);
    NameError = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("PropertyError"), &class_value);
    PropertyError = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("ValueError"), &class_value);
    ValueError = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("FatalError"), &class_value);
    FatalError = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("CompileError"), &class_value);
    CompileError = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("IOError"), &class_value);
    IOError = as_class(class_value);

    table_get(&vm.builtin, auto_length_string_copy("File"), &class_value);
    file_class = as_class(class_value);

    add_native_method(string_class, "substring", native_string_method_substring, 2);
    add_native_method(string_class, "replace", native_string_method_replace, 3);
    add_native_method(string_class, "char_at", native_string_method_char_at, 1);
    add_native_method(string_class, "strip", native_string_method_strip, 0);
    add_native_method(string_class, "f", native_string_method_format, -1);

    add_native_method(map_class, "delete", native_map_method_delete, 1);
    add_native_method(class_class, "subclass_of", native_class_method_subclass_of, 1);

    add_native_method(int_class, "hash", native_method_general_hash, 0);
    add_native_method(nil_class, "hash", native_method_general_hash, 0);
    add_native_method(float_class, "hash", native_method_general_hash, 0);
    add_native_method(bool_class, "hash", native_method_general_hash, 0);
    add_native_method(array_class, "hash", native_method_general_hash, 0);
    add_native_method(map_class, "hash", native_method_general_hash, 0);
    add_native_method(module_class, "hash", native_method_general_hash, 0);
    add_native_method(method_class, "hash", native_method_general_hash, 0);
    add_native_method(closure_class, "hash", native_method_general_hash, 0);
    add_native_method(class_class, "hash", native_method_general_hash, 0);
    add_native_method(function_class, "hash", native_method_general_hash, 0);
    add_native_method(native_class, "hash", native_method_general_hash, 0);
    add_native_method(string_class, "hash", native_method_general_hash, 0);
    add_native_method(native_object_class, "hash", native_method_general_hash, 0);

    add_native_method(int_class, "equal", native_method_value_equal, 1);
    add_native_method(nil_class, "equal", native_method_value_equal, 1);
    add_native_method(float_class, "equal", native_method_value_equal, 1);
    add_native_method(bool_class, "equal", native_method_value_equal, 1);
    add_native_method(array_class, "equal", native_method_value_equal, 1);
    add_native_method(map_class, "equal", native_method_value_equal, 1);
    add_native_method(module_class, "equal", native_method_value_equal, 1);
    add_native_method(method_class, "equal", native_method_value_equal, 1);
    add_native_method(closure_class, "equal", native_method_value_equal, 1);
    add_native_method(class_class, "equal", native_method_value_equal, 1);
    add_native_method(function_class, "equal", native_method_value_equal, 1);
    add_native_method(native_class, "equal", native_method_value_equal, 1);
    add_native_method(string_class, "equal", native_method_value_equal, 1);
    add_native_method(native_object_class, "equal", native_method_value_equal, 1);

    add_native_method(file_class, "read_chars", native_file_method_read_chars, 1);
    add_native_method(file_class, "read_line", native_file_method_read_line, 0);
    add_native_method(file_class, "read_all", native_file_method_read_all, 0);
    add_native_method(file_class, "write", native_file_method_write, -1);
    add_native_method(file_class, "close", native_file_method_close, 0);
    add_native_method(file_class, "seek", native_file_method_seek, -1);
    add_native_method(file_class, "tell", native_file_method_tell, 0);

    add_native_class_static_function(array_class, "copy", native_array_copy, 5);
    add_builtin_field("endl", ref_value_cast(auto_length_string_copy("\n")));
    preload_finished = true;
}
