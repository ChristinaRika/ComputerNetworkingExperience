#define MAX_ID_LEN 8
#define MAX_NAME_LEN 9
#define MIN_NAME_LEN 5
#define MAX_PHONE_LEN 100

#include<stdio.h>
#include"cJSON.h"
#include<stdlib.h>
#include<string.h>

typedef enum _type{STUDENT, TEACHER} Type;
typedef enum _gender{MALE, FEMALE} Gender;

struct _contacts
{
    char id[MAX_ID_LEN + 1];
    char name[MAX_NAME_LEN + 1];
    Gender gender;
    Type type;
    char mobile_phones[MAX_PHONE_LEN];
};
typedef struct _contacts Contacts;

int create_contacts_list_by_json(cJSON *root, Contacts *contacts_list, int contacts_size);
void display(Contacts *contacts_list, int contacts_size);
 
int main() //json->array->json object ->......
{
    char jsonString[2000];
    scanf("%s", jsonString);

    cJSON *root = cJSON_Parse(jsonString);
    if(!root){
        printf("init json fail.\n");
        return -1;
    }
    cJSON *contactsSize = cJSON_GetObjectItem(root, "amount");

    Contacts contacts_list[100];
    create_contacts_list_by_json(root, contacts_list, contactsSize->valueint);
    display(contacts_list, contactsSize->valueint);

    cJSON_Delete(root);
    
    return 0;
}
int create_contacts_list_by_json(cJSON *root, Contacts *contacts_list, int contacts_size){
    cJSON *student, *teacher;
    student = cJSON_GetObjectItem(root, "student");
    teacher = cJSON_GetObjectItem(root, "teacher");
    cJSON *student_contacts = cJSON_GetObjectItem(student, "contacts");
    for(int i = 0; i < cJSON_GetObjectItem(student, "amount")->valueint; i++){
        cJSON *studentItem = cJSON_GetArrayItem(student_contacts, i);   
        strcpy(contacts_list[i].id, cJSON_GetObjectItem(studentItem, "id")->valuestring);
        contacts_list[i].type = STUDENT;
        contacts_list[i].gender = cJSON_GetObjectItem(studentItem, "gender")->valueint;
        strcpy(contacts_list[i].name, cJSON_GetObjectItem(studentItem, "name")->valuestring);

        cJSON *phone = cJSON_GetObjectItem(studentItem, "mobile_phones");
        
        cJSON *item = cJSON_GetArrayItem(phone, 0);
        strcat(contacts_list[i].mobile_phones, item->valuestring);
        for(int j = 1; j < cJSON_GetArraySize(phone); j++){
            cJSON *item = cJSON_GetArrayItem(phone, j);
            strcat(contacts_list[i].mobile_phones, ",");
            strcat(contacts_list[i].mobile_phones, item->valuestring);
        }      
    }   
    cJSON *teacher_contacts = cJSON_GetObjectItem(teacher, "contacts");
    for(int i = cJSON_GetObjectItem(student, "amount")->valueint; i < cJSON_GetObjectItem(root, "amount")->valueint; i++){
        cJSON *teacherItem = cJSON_GetArrayItem(teacher_contacts, i - cJSON_GetObjectItem(student, "amount")->valueint);
        strcpy(contacts_list[i].id, cJSON_GetObjectItem(teacherItem, "id")->valuestring);
        contacts_list[i].type = TEACHER;
        contacts_list[i].gender = cJSON_GetObjectItem(teacherItem, "gender")->valueint;
        strcpy(contacts_list[i].name, cJSON_GetObjectItem(teacherItem, "name")->valuestring);

        cJSON *phone = cJSON_GetObjectItem(teacherItem, "mobile_phones");
        cJSON *item = cJSON_GetArrayItem(phone, 0);
        strcat(contacts_list[i].mobile_phones, item->valuestring);
        for(int j = 1; j < cJSON_GetArraySize(phone); j++){
            cJSON *item = cJSON_GetArrayItem(phone, j);
            strcat(contacts_list[i].mobile_phones, ",");
            strcat(contacts_list[i].mobile_phones, item->valuestring);
        } 
    }  
}
void display(Contacts *contacts_list, int contacts_size){
    for(int i = 0; i < contacts_size; i++){
        printf("%s %d %s %d %s\n",contacts_list[i].id, contacts_list[i].type,
        contacts_list[i].name, contacts_list[i].gender, contacts_list[i].mobile_phones);
    }
}