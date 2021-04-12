// Файл bn_yaremus.c
//#include "bn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string.h>

//
struct bn_s;
typedef struct bn_s bn;
enum bn_codes {
    BN_OK, BN_NULL_OBJECT, BN_NO_MEMORY, BN_DIVIDE_BY_ZERO
};

bn* bn_new(); // Со�дат� новое BN
bn* bn_init(bn const* orig); // Со�дат� копи� существу�щего BN
// Инициали�ироват� �начение BN дес�тичным представлением строки
int bn_init_string(bn* t, const char* init_string);

// Инициали�ироват� �начение BN представлением строки
// в системе счислени� radix
int bn_init_string_radix(bn* t, const char* init_string, int radix);

int bn_init_int(bn* t, int init_int);

// Уничто�ит� BN (освободит� пам�т�)
int bn_delete(bn* t);

// Операции, аналогичные +=, -=, *=, /=, %=
int bn_add_to(bn* t, bn const* right);

int bn_sub_to(bn* t, bn const* right);

int bn_mul_to(bn* t, bn const* right);

int bn_div_to(bn* t, bn const* right);

int bn_mod_to(bn* t, bn const* right);

// Во�вести число в степен� degree
int bn_pow_to(bn* t, int degree);

// И�влеч� корен� степени reciprocal и� BN (бонусна� функци�)
int bn_root_to(bn* t, int reciprocal);

// Аналоги операций x = l+r (l-r, l*r, l/r, l%r)
bn* bn_add(bn const* left, bn const* right);

bn* bn_sub(bn const* left, bn const* right);

bn* bn_mul(bn const* left, bn const* right);

bn* bn_div(bn const* left, bn const* right);

bn* bn_mod(bn const* left, bn const* right);

// Выдат� представление BN в системе счислени� radix в виде строки
// Строку после испол��овани� потребуетс� удалит�.
const char* bn_to_string(bn const* t, int radix);

// Если левое мен�ше, вернут� <0; если равны, вернут� 0; иначе >0
int bn_cmp(bn const* left, bn const* right);

// Если модуль левого меньше, вернуть -1, если равны, вернуть 0, иначе 1
int bn_cmp_abs(bn const* left, bn const* right);

int bn_neg(bn* t); // И�менит� �нак на противополо�ный
int bn_abs(bn* t); // В��т� модул�
int bn_sign(bn const* t); //-1 если t<0; 0 если t = 0, 1 если t>0
//
#define max(a, b) ((a) > (b) ? (a) : (b))

const int base = 1000000000;

struct bn_s {
    int* body; // Тело бол�шого числа
    int size; // Ра�мер массива body
    int sign; //0 �нак числа
    int allocated; //сколько памяти выделено sizeof(int)
};

int bn_realloc(bn* t,int size) {
    t->allocated = (int)sizeof(int) * size * 2;
    t->body = (int*)realloc(t->body, t->allocated);
    if (t->body == NULL) {
        free(t);
        return BN_NO_MEMORY;
    }
    for (int i = t->size; i < (int)(t->allocated / sizeof(int)); i++) {
        t->body[i] = 0;
    }
    return 0;
}


// Со�дат� новое BN
bn* bn_new() {
    bn* r = malloc(sizeof(bn));
    if (r == NULL) return NULL;
    r->size = 1;
    r->sign = 0;
    r->allocated = (int)sizeof(int) * r->size * 2;
    r->body = (int*)malloc(r->allocated);
    if (r->body == NULL) {
        free(r);
        return NULL;
    }
    r->body[0] = 0;
    r->body[1] = 0;
    return r;
}

// Со�дат� копи� существу�щего BN
bn* bn_init(bn const* orig) {
    bn* B = malloc(sizeof(bn));
    if (B == NULL) return NULL;
    B->allocated = orig->allocated;
    B->body = malloc(orig->allocated);
    if (B->body == NULL) {
        free(B);
        return NULL;
    }
    B->sign = orig->sign;
    B->size = orig->size;
    for (int i = 0; i < (int)(orig->allocated/sizeof(int)); i++) {
        B->body[i] = orig->body[i];
    }
    return B;
}

int bn_normalize(bn *t) {
    if (t->body[0] == 0 && t->size == 1) {
        t->sign = 0;
        return 0;
    }
    int i = 0;
    while (t->body[(int)(t->allocated/sizeof(int)) - i - 1] == 0 && i < (int)(t->allocated/sizeof(int))) i++;
    t->size = (int)(t->allocated/sizeof(int)) - i;
    bn_realloc(t, t->size);
    if ((int)(t->allocated/sizeof(int) > t->size)) {
        for (int i = t->size; i < (int) (t->allocated / sizeof(int)); i++) {
            t->body[i] = 0;
        }
    }
    return 0;
}


int bn_neg(bn* t) {
    if (t->sign == 0) {
        return 0;
    }
    t->sign *= -1;
    return 0;
}

int bn_abs(bn* t) {
    if (t->sign == 0) {
        return 0;
    }
    t->sign = 1;
    return 0;
}

int bn_sign(bn const* t){
    if (t == NULL) return BN_NULL_OBJECT;
    return t->sign;
}

int bn_copy(bn* t, bn const * f) {
    t->allocated = f->allocated;
    t->body = (int*)realloc(t->body, f->allocated);
    t->size = f->size;
    t->sign = f->sign;
    if (t->body == NULL) return BN_NO_MEMORY;
    for (int i = 0; i < (int)(t->allocated/sizeof(int)); i++) {
        t->body[i] = f->body[i];
    }
    return 0;
}

//Инициали�ироват� �начение BN дес�тичным представлением строки
int bn_init_string(bn* t, const char* init_string) {
    if (t == NULL) return BN_NULL_OBJECT;
    if (init_string == NULL) return BN_NULL_OBJECT;
    int str_len = (int)strlen(init_string);//длина строки
    // инициализация знака, размера числа и вычисление длины используемой строки
    if (*init_string == '-') {
        t->sign = -1;
        init_string++;
        str_len--;
    }
    else if (*init_string == '0' && str_len == 1) {
        t->size = 1;
        t->sign = 0;
        t->allocated = (int)sizeof(int) * t->size * 2;
        if (t->body != NULL) free(t->body);
        t->body = malloc(t->allocated);
        if (t->body == NULL) return BN_NULL_OBJECT;
        t->body[0] = 0;
        return 0;
    }
    else {
        t->sign = 1;
    }
    t->size = (int)((str_len + 8) / 9);
    int body = 0;
    // заполнение body
    bn_realloc(t, t->size);
    char temp[10] = {};
    for (int i = str_len - 1; i >= 0; i -= 9) {
        if (i >= 9) {
            for (int j = 0; j < 9; j++) {
                temp[j] = init_string[i - 8 + j];
            }
            temp[9] = '\0';
            t->body[body++] = (int)strtol(temp, NULL, 10);
        }
        else {
            int j = 0;
            while (j < i + 1) {
                temp[j] = init_string[j];
                j++;
            }
            if (j < 9) {
                temp[j] = '\0';
            }
            temp[j] = '\0';
            t->body[body++] = (int)strtol(temp, NULL, 10);
        }
    }
    bn_normalize(t);
    return 0;
}



void bn_vivod_dec(bn* t) {
    printf("body:%d", t->body[t->size - 1]);
    for (int i = 1; i < t->size; i++) {
        printf("%09d", t->body[t->size - i - 1]);
    }
    printf("\nbodysize: %d", t->size);
    printf("\nsign: %d", t->sign);
    printf("\nallocated: %d\n", t->allocated);
}

void bn_vivod(bn* t) {
    printf("body:");
    for (int i = 0; i < t->size; i++) {
        printf("%d.", t->body[i]);
    }
    printf("\nbodysize: %d", t->size);
    printf("\nsign: %d", t->sign);
    printf("\nallocated: %d\n", t->allocated);
}

int bn_init_int(bn* t, int init_int) {
    //орпеделение знака
    if (init_int < 0) {
        t->sign = -1;
    }
    else if (init_int == 0) {
        t->size = 1;
        t->sign = 0;
        t->allocated = (int)sizeof(int) * t->size * 2;
        t->body = (int*)realloc(t->body, t->allocated);
        if (t->body == NULL) return BN_NULL_OBJECT;
        t->body[0] = 0;
        return 0;
    }
    else {
        t->sign = 1;
    }
    int num = init_int;
    int iter = 0;
    while (num != 0) {
        num = num / 10;
        iter++;
    }
    t->size = ((iter + 8) / 9);
    int i = 0;
    bn_realloc(t, t->size);
    while (init_int != 0) {
        t->body[i++] = abs(init_int % base);
        init_int = (int)(init_int / base);
    }
    bn_normalize(t);
    return 0;
}

int bn_delete(bn* t) {
    if (t == NULL) return BN_NULL_OBJECT;
    free(t->body);
    free(t);
    return 0;
}

int bn_cmp(bn const* left, bn const* right) {
    if (left->sign > right->sign) return 1;
    else if (left->sign < right->sign) return -1;
    else if (left->sign == 0) return 0;
    if (left->size > right->size) return left->sign;
    else if (left->size < right->size) return (-1) * left->sign;
    int i = 0;
    while (i != left->size) {
        if (left->body[left->size - 1 - i] > right->body[right->size - 1 - i]) return left->sign;
        else if (left->body[left->size - 1 - i] < right->body[right->size - 1 - i]) return (-1) * left->sign;
        i++;
    }
    return 0;
}

int bn_cmp_abs(bn const* left, bn const* right) {
    if (left->size > right->size) return 1;
    else if (left->size < right->size) return -1;
    int i = 0;
    while (i != left->size) {
        if (left->body[left->size - 1 - i] > right->body[right->size - 1 - i]) return 1;
        else if (left->body[left->size - 1 - i] < right->body[right->size - 1 - i]) return -1;
        i++;
    }
    return 0;
}


int bn_add_to_abs(bn* t, bn const* right) {
    int maxx = max(t->size, right->size);
    int c = 0;
    int dobavka = 0;
    if (t->allocated <= (sizeof(int) * maxx)) {
        bn_realloc(t, maxx);
    }
    for (int i = 0; (i < right->size) || c; i++) {
        if (i == t->size) t->size++;
        if (i < right->size) dobavka = right->body[i];
        else dobavka = 0;
        t->body[i] += c + dobavka;
        c = (int)t->body[i] / base;
        if (c == 1) t->body[i] -= base;
    }
    bn_normalize(t);
    return 0;
}

int bn_sub_to_abs(bn* t, bn const* right) {
    int maxx = max(t->size, right->size);
    int c = 0;
    int dobavka = 0;
    if (t->allocated < (int)(sizeof(int) * maxx)) {
        bn_realloc(t, maxx);
    }
    int f = bn_cmp_abs(t, right);
    if (f == 0) {
        t->allocated = (int)sizeof(int) * 2;
        t->body = (int*)realloc(t->body, t->allocated);
        t->body[0] = 0;
        t->body[1] = 0;
        t->sign = 0;
        t->size = 1;
        return 0;
    } else if (f == 1) {
        for (int i = 0; i < right->size || c ; i++) {
            if (i < right->size) dobavka = right->body[i];
            else dobavka = 0;
            t->body[i] -= (dobavka + c);
            c = t->body[i] < 0;
            if (c) t->body[i] += base;
        }
    } else {
        int raz = 0;
        for (int i = 0; i < t->size || c ; i++) {
            if (i < t->size) dobavka = t->body[i];
            else dobavka = 0;
            t->body[i] = right->body[i] - (dobavka + c);
            c = t->body[i] < 0;
            if (c) t->body[i] += base;
            if (i >= t->size) raz++;
        }
        for (int i = t->size + raz; i < right->size; i++) {
            t->body[i] = right->body[i];
            t->size++;
        }
    }
    bn_normalize(t);
    return 0;
}


int bn_add_to(bn* t, bn const* right) {
    if (t->sign == 0) {
        bn_copy(t, right);
        return 0;
    }
    if (right->sign == 0) return 0;
    if (t->sign * right->sign == 1) {
        bn_add_to_abs(t, right);
        bn_normalize(t);
        return 0;
    }
    t->sign = bn_cmp(t, right) * bn_cmp_abs(t, right);
    bn_sub_to_abs(t, right);
    bn_normalize(t);
    return 0;
}

bn* bn_add(bn const* left, bn const* right) {
    bn* res = bn_new();
    if (left->sign == 0 || right->sign == 0) {
        bn_copy(res, left->sign == 0 ? right : left);
    }
    else if (left->sign * right->sign == 1) {
        bn_add_to_abs(res, left);
        bn_add_to_abs(res, right);
        res->sign = left->sign;
    }
    else {
        bn_add_to_abs(res, left);
        bn_sub_to_abs(res, right);
        res->sign = bn_cmp(left, right) * bn_cmp_abs(left, right);
    }
    bn_normalize(res);
    return res;
}

int bn_sub_to(bn* t, bn const* right) {
    if (t->sign == 0) {
        bn_copy(t, right);
        t->sign = right->sign*(-1);
        return 0;
    }
    if (right->sign == 0) return 0;
    if (t->sign * right->sign == 1) {
        t->sign = right->sign * bn_cmp_abs(t, right);
        bn_sub_to_abs(t, right);
        bn_normalize(t);
        return 0;
    }
    bn_add_to_abs(t, right);
    bn_normalize(t);
    return 0;
}

bn* bn_sub(bn const* left, bn const* right) {
    bn* res = bn_new();
    if (left->sign == 0 || right->sign == 0) {
        bn_copy(res, left->sign == 0 ? right : left);
        res->sign = left->sign == 0 ? right->sign * (-1) : left->sign;
    }
    else if (left->sign * right->sign == 1) {
        bn_add_to_abs(res, left);
        bn_sub_to_abs(res, right);
        res->sign = right->sign * bn_cmp_abs(left, right);
    }
    else {
        bn_add_to_abs(res, left);
        bn_add_to_abs(res, right);
        res->sign = left->sign;
    }
    bn_normalize(res);
    return res;
}

int bn_input(bn* t) {
    int size = 256;
    char* ch = malloc(sizeof(char)*size);
    int i = 0;
    while((ch[i]=(char)getchar()) != '\n') {
        if (i == size - 1) {
            size *= 2;
            ch = (char*)realloc(ch, sizeof(char) * size);
        }
        i++;
    }
    ch[i] = '\0'; /* добавление нулевого символа */
    bn_init_string(t, ch);
    bn_normalize(t);
    free(ch);
    return 0;
}
int bn_add_long(bn* t, long ch, int s) {
    int c = 0;
    long slag = ch;
    long temp = 0;
    for (int i = s; c != 0 || slag != 0; i++) {
        temp = t->body[i] + slag % base + c;
        t->body[i] = (int)(temp % base);
        slag /= base;
        c = (int)(temp / base);
    }
    return 0;
}


int bn_mul_to(bn* t, bn const* right) {
    if (t->sign == 0 || right->sign == 0) {
        t->sign = 0;
        t->size = 1;
        bn_realloc(t, t->size);
        t->body[0] = 0;
        t->body[1] = 0;
        return 0;
    }
    bn* t2 = bn_new();
    bn_realloc(t2, t->size + right->size);
    t2->sign = t->sign * right->sign;
    bn_realloc(t, t->size + right->size);
    t2->size = t->size + right->size;
    for (int i = 0; i < t->size; i++) {
        for (int j = 0; j < right->size; j++) {
            long temp = (long)t->body[i] * (long)right->body[j];
            bn_add_long(t2, temp, i + j);
        }
    }
    bn_copy(t, t2);
    bn_delete(t2);
    bn_normalize(t);
    return 0;
}

int bn_short_mul(bn* t, int num) {
    if (t->sign == 0 || num == 0) {
        t->sign = 0;
        t->size = 1;
        bn_realloc(t, t->size);
        t->body[0] = 0;
        t->body[1] = 0;
        return 0;
    }
    bn* t2 = bn_new();
    t2->size = t->size + 1;
    bn_realloc(t2, t2->size);
    t2->sign = t->sign * (num > 0 ? 1 : -1);
    bn_realloc(t, t->size + 1);
    for (int i = 0; i < t->size; i++) {
        long temp = (long)t->body[i] * (long)abs(num);
        bn_add_long(t2, temp, i);
    }
    bn_copy(t, t2);
    bn_delete(t2);
    bn_normalize(t);
    return 0;
}

bn* bn_mul(bn const* left, bn const* right) {
    bn* t = bn_init(left);
    bn_mul_to(t, right);
    return t;
}

int bn_pow_to(bn* t, int degree) {
    if (t->body[0] == 1 && t->size == 1) {
        if (t->sign == -1 && degree % 2 == 0)
            t->sign = 1;
        return 0;
    }
    if (degree == 0) {
        bn_init_int(t, 1);
        return 0;
    }
    bn* temp = bn_new();
    bn_copy(temp, t);
    for (int i = 0; i < degree - 1; i++) {
        bn_mul_to(t, temp);
    }
    bn_delete(temp);
    return 0;
}

bn* bn_div(bn const* left, bn const* right) {
    if (left == NULL || right == NULL)
        return NULL;
    if (right->sign == 0)
        return NULL;

    if (bn_cmp_abs(left, right) == -1)
        return bn_new();
    if (bn_cmp_abs(left, right) == 0) {
        bn *ans = bn_new();
        bn_init_int(ans, left->sign * right->sign);
        return ans;
    }

    bn* ost = bn_init(left);
    bn_abs(ost);
    bn* des = bn_new();
    bn* answer = bn_new();
    bn *digit = bn_new();

    int length = left->size > right->size ? left->size - right->size + 1 : 1;
    int *ans = malloc(length * sizeof (int));
    for (int i = 0; i < length; i++) ans[i] = 0;
    int pos = 0;
    while (bn_cmp_abs(ost, right) > 0) {
        bn* compare = bn_new();
        bn_init_int(des, base);

        bn_pow_to(des, length - pos - 1);

        int l = -1;
        int s = 0;
        int r = base;

        while (r - l > 1) {
            s = (int)((r + l) / 2);
            bn_init_int(digit, s);
            bn* temp = bn_mul(right, des);
            bn_copy(compare, temp);
            bn_delete(temp);
            bn_mul_to(compare, digit);

            if (bn_cmp_abs(ost, compare) > 0)
                l = s;
            else if (bn_cmp_abs(ost, compare) < 0)
                r = s;
            else {
                l = s;
                break;
            }
        }
        bn_init_int(digit, l);

        des->sign = right->sign;
        bn* temp = bn_mul(right, des);
        bn_copy(compare, temp);
        bn_delete(temp);
        bn_mul_to(compare, digit);

        bn_sub_to(ost, compare);
        bn_normalize(ost);

        ans[pos] = l;
        pos += 1;
        bn_delete(compare);
    }
    answer->size = length;
    bn_realloc(answer, length);
    for (int i = 0; i < length; i++) {
        answer->body[length - i - 1] = ans[i];
    }
    answer->sign = left->sign == right->sign ? 1 : -1;
    bn_normalize(ost);
    if (left->sign * right->sign < 0 && ost->sign != 0) bn_add_long(answer, 1, 0);
    bn_normalize(answer);
    free(ans);
    bn_delete(ost);
    bn_delete(des);
    bn_delete(digit);
    return answer;
}

int bn_short_div(bn* t, int del) {
    if (t == NULL)
        return BN_NULL_OBJECT;
    if (del == 0)
        return BN_DIVIDE_BY_ZERO;
    bn_normalize(t);
    long ost = 0;
    for (int i = t->size - 1; i >= 0; i--) {
        long temp = (long)t->body[i];
        temp += base * ost;
        t->body[i] = (int)(temp / del);
        ost = (temp) % del;
    }
    t->sign = t->sign * (del > 0 ? 1 : -1);
    bn_normalize(t);
    return 0;
}

int bn_short_mod(bn* t, int del) {
    if (t == NULL)
        return BN_NULL_OBJECT;
    if (del == 0)
        return BN_DIVIDE_BY_ZERO;
    bn_normalize(t);
    long ost = 0;
    for (int i = t->size - 1; i >= 0; i--) {
        long temp = (long)t->body[i];
        temp += base * ost;
        t->body[i] = (int)(temp / del);
        ost = (temp) % del;
    }
    return ost;
}

int bn_div_by_2(bn* t) {
    if (t == NULL)
        return BN_NULL_OBJECT;
    bn_normalize(t);
    long ost = 0;
    for (int i = t->size - 1; i >= 0; i--) {
        long temp = (long)t->body[i];
        temp += base * ost;
        t->body[i] = (int)(temp >> 1);
        ost = (temp & 1);
    }
    bn_normalize(t);
    return 0;
}

int bn_div_to(bn* t, bn const* right) {
    if (t == NULL || right == NULL)
        return BN_NULL_OBJECT;
    if (right->sign == 0)
        return BN_DIVIDE_BY_ZERO;
    bn* temp = bn_div(t, right);
    bn_copy(t, temp);
    bn_delete(temp);
    return 0;
}

bn* bn_mod(bn const* left, bn const* right) {
    if (left == NULL || right == NULL)
        return NULL;
    if (right->sign == 0)
        return NULL;

    if (bn_cmp_abs(left, right) == -1) {
        bn * temp = bn_new();
        bn_copy(temp, left);
        return temp;
    }
    if (bn_cmp_abs(left, right) == 0) {
        bn *ans = bn_new();
        return ans;
    }

    bn* ost = bn_init(left);
    bn_abs(ost);
    bn* des = bn_new();
    bn *digit = bn_new();

    int length = left->size > right->size ? left->size - right->size + 1 : 1;
    int pos = 0;
    while (bn_cmp_abs(ost, right) > 0) {
        bn* compare = bn_new();
        bn_init_int(des, base);

        bn_pow_to(des, length - pos - 1);

        int l = -1;
        int s = 0;
        int r = base;

        while (r - l > 1) {
            s = (int)((r + l) / 2);
            bn_init_int(digit, s);
            bn* temp = bn_mul(right, des);
            bn_copy(compare, temp);
            bn_delete(temp);
            bn_mul_to(compare, digit);

            if (bn_cmp_abs(ost, compare) > 0)
                l = s;
            else if (bn_cmp_abs(ost, compare) < 0)
                r = s;
            else {
                l = s;
                break;
            }
        }
        bn_init_int(digit, l);

        des->sign = right->sign;
        bn* temp = bn_mul(right, des);
        bn_copy(compare, temp);
        bn_delete(temp);
        bn_mul_to(compare, digit);

        bn_sub_to(ost, compare);
        bn_normalize(ost);

        pos += 1;
        bn_delete(compare);
    }
    bn_normalize(ost);
    bn_delete(des);
    bn_delete(digit);
    if (right->sign * left->sign == 1){
        ost->sign = right->sign;
        return ost;
    }
        bn* temp = bn_init(right);
        temp->sign = ost->sign * (-1);
        bn_add_to(temp, ost);
        temp->sign = right->sign;
        bn_normalize(temp);
        bn_delete(ost);
        return temp;
}

int bn_mod_to(bn* t, bn const* right) {
    if (t == NULL || right == NULL)
        return BN_NULL_OBJECT;
    if (right->sign == 0)
        return BN_DIVIDE_BY_ZERO;
    bn* temp = bn_mod(t, right);
    bn_copy(t, temp);
    bn_delete(t);
    return 0;
}

int bn_root_to(bn* t, int reciprocal) {
    int len = (int)(t->size/reciprocal);
    bn* root;
    bn* left = bn_new();
    bn* right = bn_new();
    bn* one = bn_new();
    bn* temp = bn_new();
    bn* delta;

    bn_init_int(one, 1);
    left->size = len < 2 ? 1: len - 1;
    bn_realloc(left, left->size);
    right->size = len + 1;
    bn_realloc(right, right->size);


    for (int i = 0; i < left->size; i++) {
        left->body[i] = 0;
    }
    left->body[0] = 0;
    for (int i = 0; i < right->size; i++) {
        right->body[i] = base - 1;
    }
    right->sign = 1;

    while (bn_cmp_abs(delta = bn_sub(right, left), one) != 0) {
        bn_delete(delta);
        root = bn_add(left, right);
        //bn_short_div(root, 2);
        bn_div_by_2(root);
        bn_copy(temp, root);
        bn_pow_to(temp, reciprocal);
        if (bn_cmp(t, temp) >= 0) {
            bn_copy(left, root);
        } else {
            bn_copy(right, root);
        }
        bn_delete(root);
    }
    bn_copy(t, left);
    bn_delete(left);
    bn_delete(right);
    bn_delete(temp);
    bn_delete(one);
    bn_delete(delta);
    return 0;
}

// Инициализировать значение BN представлением строки
// в системе счисления radix от 2 до 36
int bn_init_string_radix(bn *t, const char *init_string, int radix) {
    int len = (int)strlen(init_string);
    if (len == 0)
        return BN_NULL_OBJECT;
    t->size = 1;
    bn_realloc(t, (len/9 + 1));
    t->sign = 1;
    t->body[0] = 0;
    if (init_string[0] == '-') {
        t->sign = -1;
        init_string++;
    }
    int tmp = 0;
    for (int i = 0; i < len; ++i) {
        if (i > 0)
            bn_short_mul(t, radix);
        if (init_string[i] - '0' < 10 && init_string[i] >= '0') {
            tmp = init_string[i] - '0';
        } else {
            tmp = init_string[i] - 'A' + 10;
        }
        bn_add_long(t, tmp, 0);
    }
    bn_normalize(t);
    if (t->body[0] == 0 && t->size == 1)
        t->sign = 0;
    return 0;
}

const char* bn_to_string(bn const* t, int radix) {
    if (t == NULL)
        return NULL;
    if (t->body == NULL)
        return NULL;
    char *c = malloc(sizeof(char) * t->size * 9 * 5);
    if (c == NULL)
        return NULL;
    int i = 0;
    c[i++] = '\0';
    bn* copy = bn_init(t);
    while ((copy->size > 1) || (copy->body[0] > 0)) {
        c[i++] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[bn_short_mod(copy, radix)];
    }
    char temp;
    if (t->sign == -1)
        c[i++] = '-';
    for (int j = 0; j < i/2; j++) {
        temp = c[j];
        c[j] = c[i - j - 1];
        c[i - j - 1] = temp;
    }
    if (c[0] == 0) {
        c[0] = '0';
        c[1] = '\0';
    }
    bn_delete(copy);
    return c;
}

//int main() {
//    bn* a = bn_new();
//    bn* b = bn_new();
//    bn* c;
//    bn_input(a);
//    int s = 0;
//    s = getchar();
//    getchar();
//    bn_input(b);
//    if (s == '/') {
//        c = bn_div(a, b);
//    }
//    if (c->sign == -1) printf("-");
//    printf("%d", c->body[c->size - 1]);
//    for (int i = 1; i < c->size; i++) {
//        printf("%09d", c->body[c->size - i - 1]);
//    }
//    bn_delete(c);
//    bn_delete(a);
//    bn_delete(b);
//    return 0;
//}