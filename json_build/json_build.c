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

int contacts_num = 0;
int male_num = 0;
int female_num = 0;
int student_num = 0;
int student_male_num = 0;
int student_female_num = 0;
int teacher_num = 0;
int teacher_male_num = 0;
int teacher_female_num = 0;

struct _contacts
{
    char id[MAX_ID_LEN + 1];
    char name[MAX_NAME_LEN + 1];
    Gender gender;
    Type type;
    char mobile_phones[MAX_PHONE_LEN];         //if I use char*, then I need to use link list
};
typedef struct _contacts Contacts;

void insert(cJSON *root, Contacts *contacts);
void getContacts(Contacts *contacts);
void calculateNum(Contacts *contacts);

void main(){
    Contacts contacts[100];
    cJSON *root;

    getContacts(contacts);
    calculateNum(contacts);

    root = cJSON_CreateObject();   
    insert(root, contacts);

    puts(cJSON_PrintUnformatted(root));

    cJSON_Delete(root);
}

void getContacts(Contacts *contacts){
    char ch;
    while((ch = getchar()) != '\n'){
        ungetc(ch, stdin);
        scanf("%s",contacts[contacts_num].id);
        scanf("%d",&(contacts[contacts_num].type));
        scanf("%s",contacts[contacts_num].name);
        scanf("%d",&(contacts[contacts_num].gender));
        scanf("%s",contacts[contacts_num].mobile_phones);
        contacts_num++;
        getchar();
    }
}

void calculateNum(Contacts *contacts){
    for(int i = 0; i < contacts_num; i++){
        if(contacts[i].gender == MALE){
            male_num ++;
            if(contacts[i].type == STUDENT)
                student_male_num++;
        }
        if(contacts[i].type == STUDENT){
            student_num++;
        }
    }
    female_num = contacts_num - male_num;
    teacher_num = contacts_num - student_num;
    student_female_num = student_num - student_male_num;
    teacher_male_num = male_num - student_male_num;
    teacher_female_num = teacher_num - teacher_male_num;
}

void insert(cJSON *root, Contacts *contacts){
    cJSON *student, *teacher;
    cJSON_AddNumberToObject(root, "amount", contacts_num);

    cJSON_AddItemToObject(root, "student", student = cJSON_CreateObject());
    cJSON_AddItemToObject(root, "teacher", teacher = cJSON_CreateObject());  
    //student
    cJSON_AddNumberToObject(student, "amount", student_num);
    int std_male_ratio = (int)(((float)student_male_num/(float)student_num) * 100 + 0.5);
    int std_female_ratio = 100 - std_male_ratio;
    char stdMaleRatio[5], stdFemaleRatio[5];
    sprintf(stdMaleRatio,"%d%%", std_male_ratio);
    sprintf(stdFemaleRatio,"%d%%", std_female_ratio);
    cJSON_AddStringToObject(student, "male_ratio", stdMaleRatio);
    cJSON_AddStringToObject(student, "female_ratio", stdFemaleRatio);
    //teacher
    cJSON_AddNumberToObject(teacher, "amount", teacher_num);
    int th_male_ratio = (int)(((float)teacher_male_num/(float)teacher_num) * 100 + 0.5);
    int th_female_ratio = 100 - th_male_ratio;
    char thMaleRatio[5], thFemaleRatio[5];
    sprintf(thMaleRatio,"%d%%", th_male_ratio);
    sprintf(thFemaleRatio,"%d%%", th_female_ratio);
    cJSON_AddStringToObject(teacher, "male_ratio", thMaleRatio);
    cJSON_AddStringToObject(teacher, "female_ratio", thFemaleRatio);
    
    cJSON *student_contacts, *teacher_contacts;
    cJSON_AddItemToObject(student, "contacts", student_contacts = cJSON_CreateArray());
    cJSON_AddItemToObject(teacher, "contacts", teacher_contacts = cJSON_CreateArray());

    for(int i = 0; i < contacts_num; i++){
        cJSON *tempObj;
        int male_ratio, female_ratio;
        tempObj = cJSON_CreateObject();
        if(contacts[i].type == STUDENT){
            cJSON_AddItemToArray(student_contacts, tempObj);
        }else{
            cJSON_AddItemToArray(teacher_contacts, tempObj);
        }
        cJSON_AddStringToObject(tempObj, "id", contacts[i].id);
        cJSON_AddStringToObject(tempObj, "name", contacts[i].name);
        cJSON_AddNumberToObject(tempObj, "gender", contacts[i].gender);
        
        cJSON *mobile;
        cJSON_AddItemToObject(tempObj, "mobile_phones", mobile = cJSON_CreateArray());
        char delims[] = ",";
        char *phone_number;
        phone_number = strtok(contacts[i].mobile_phones, delims);
        while( phone_number != NULL ) {  
            cJSON_AddItemToArray(mobile, cJSON_CreateString(phone_number));  
            phone_number = strtok( NULL, delims );  
        }          
    }
}
