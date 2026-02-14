// hospital_utf8.c
// Complete program: console UTF-8 + CSV (UTF-8 with BOM) storage
// Build on Windows (MSVC / MinGW)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>     // for _getch()
#include <windows.h>

#define MAX_PATIENTS 100
#define MAX_DOCTORS  50
#define SCREEN_WIDTH 80
#define MAX_PASSWORD_LEN 100

/* Data folder and filenames (UTF-8 CSV) */
const char DATA_FOLDER[]   = "C:\\HospitalData";
const char PATIENTS_UTF8[] = "C:\\HospitalData\\patients.csv";
const char DOCTORS_UTF8[]  = "C:\\HospitalData\\doctors.csv";

/* Doctor password (change as needed) */
const char DOCTOR_PASSWORD[] = "admin123";

typedef struct {
    int id;
    char name[100];
    int age;
    char disease[100];
} Patient;

typedef struct {
    int id;
    char name[100];
    char specialty[100];
} Doctor;

Patient patients[MAX_PATIENTS];
int patientCount = 0;

Doctor doctors[MAX_DOCTORS];
int doctorCount = 0;

/* Forward declarations */
void loadData(void);
void saveData(void);
void printCenter(const char *text);
int authenticateDoctor(void);
void addPatient(void);
void viewPatients(void);
void editPatient(void);
void deletePatient(void);
void searchPatient(void);
void addDoctor(void);
void viewDoctors(void);
void editDoctor(void);
void deleteDoctor(void);
void searchDoctor(void);
void mainMenu(void);
void centerText(const char *text);

/* ------------------ UTILITY ------------------ */

void printCenter(const char *text) {
    int len = (int)strlen(text);
    int spaces = (SCREEN_WIDTH - len) / 2;
    if (spaces < 0) spaces = 0;
    for (int i = 0; i < spaces; i++) putchar(' ');
    printf("%s", text);
}

/* Write a UTF-8 BOM to the start of a file so editors detect UTF-8 */
static void write_utf8_bom(FILE *f) {
    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    fwrite(bom, 1, sizeof(bom), f);
}

/* ------------------ FILE HANDLING (UTF-8 CSV) ------------------ */

/*
  Note: This is a simple CSV format: id,name,age,disease for patients
        and id,name,specialty for doctors.
  It does NOT implement full CSV quoting/escaping. If you need embedded commas or newlines
  inside fields, ask and I'll provide proper quoting or a JSON storage implementation.
*/

void loadData() {
    patientCount = 0;
    doctorCount = 0;

    /* Load patients CSV */
    FILE *fp = fopen(PATIENTS_UTF8, "rb");
    if (fp) {
        /* Skip BOM if present */
        unsigned char bom[3];
        if (fread(bom, 1, 3, fp) == 3) {
            if (!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
                /* No BOM, rewind */
                fseek(fp, 0, SEEK_SET);
            }
        } else {
            fseek(fp, 0, SEEK_SET);
        }

        char line[512];
        while (fgets(line, sizeof(line), fp) && patientCount < MAX_PATIENTS) {
            /* CSV format (simple): id,name,age,disease
               We ignore the stored id and reassign sequential IDs in memory. */
            Patient p;
            char name[200] = {0}, disease[200] = {0};
            int age = 0;
            /* Try to parse: allow that disease may be empty */
            int parsed = sscanf(line, "%*d,%199[^,],%d,%199[^\r\n]", name, &age, disease);
            if (parsed >= 2) {
                strncpy(p.name, name, sizeof(p.name) - 1); p.name[sizeof(p.name)-1] = '\0';
                p.age = age;
                if (parsed == 3) {
                    strncpy(p.disease, disease, sizeof(p.disease) - 1); p.disease[sizeof(p.disease)-1] = '\0';
                } else {
                    p.disease[0] = '\0';
                }
                p.id = patientCount + 1;
                patients[patientCount++] = p;
            }
        }
        fclose(fp);
    }

    /* Load doctors CSV */
    fp = fopen(DOCTORS_UTF8, "rb");
    if (fp) {
        unsigned char bom[3];
        if (fread(bom, 1, 3, fp) == 3) {
            if (!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
                fseek(fp, 0, SEEK_SET);
            }
        } else {
            fseek(fp, 0, SEEK_SET);
        }

        char line[512];
        while (fgets(line, sizeof(line), fp) && doctorCount < MAX_DOCTORS) {
            Doctor d;
            char name[200] = {0}, specialty[200] = {0};
            int parsed = sscanf(line, "%*d,%199[^,],%199[^\r\n]", name, specialty);
            if (parsed >= 1) {
                strncpy(d.name, name, sizeof(d.name) - 1); d.name[sizeof(d.name)-1] = '\0';
                if (parsed == 2) {
                    strncpy(d.specialty, specialty, sizeof(d.specialty) - 1); d.specialty[sizeof(d.specialty)-1] = '\0';
                } else {
                    d.specialty[0] = '\0';
                }
                d.id = doctorCount + 1;
                doctors[doctorCount++] = d;
            }
        }
        fclose(fp);
    }
}

void saveData() {
    /* Save patients as UTF-8 CSV */
    FILE *fp = fopen(PATIENTS_UTF8, "wb");
    if (fp) {
        write_utf8_bom(fp);
        for (int i = 0; i < patientCount; i++) {
            /* id,name,age,disease\n */
            /* NOTE: fields are NOT escaped. Keep commas out of names/diseases or request escaping. */
            fprintf(fp, "%d,%s,%d,%s\n",
                    patients[i].id,
                    patients[i].name,
                    patients[i].age,
                    patients[i].disease);
        }
        fclose(fp);
    } else {
        printCenter("Warning: could not save patients data (patients.csv).\n");
    }

    /* Save doctors as UTF-8 CSV */
    fp = fopen(DOCTORS_UTF8, "wb");
    if (fp) {
        write_utf8_bom(fp);
        for (int i = 0; i < doctorCount; i++) {
            fprintf(fp, "%d,%s,%s\n",
                    doctors[i].id,
                    doctors[i].name,
                    doctors[i].specialty);
        }
        fclose(fp);
    } else {
        printCenter("Warning: could not save doctors data (doctors.csv).\n");
    }
}

/* ------------------ AUTHENTICATION ------------------ */

/* Prompt for password (masked). Returns 1 if correct, 0 otherwise. */
int authenticateDoctor() {
    char input[MAX_PASSWORD_LEN];
    int idx = 0;
    int ch;

    printCenter("Enter doctor password: ");

    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == 8) { // backspace
            if (idx > 0) {
                idx--;
                printf("\b \b");
            }
        } else if (ch >= 32 && idx < (MAX_PASSWORD_LEN - 1)) {
            input[idx++] = (char)ch;
            printf("*");
        }
    }
    input[idx] = '\0';
    printf("\n");

    if (strcmp(input, DOCTOR_PASSWORD) == 0) {
        printCenter("Access granted.\n");
        return 1;
    } else {
        printCenter("Access denied. Invalid password.\n");
        return 0;
    }
}

/* ------------------ PATIENT FUNCTIONS ------------------ */

void addPatient() {
    if (patientCount >= MAX_PATIENTS) {
        printCenter("Patient list is full.\n");
        return;
    }
    Patient p;
    p.id = patientCount + 1;

    /* Use fgets for safer input (trim trailing newline) */
    printCenter("Enter patient name: ");
    fflush(stdout);
    fgets(p.name, sizeof(p.name), stdin);
    if (p.name[strlen(p.name)-1] == '\n') p.name[strcspn(p.name, "\r\n")] = '\0';
    if (strlen(p.name) == 0) {
        printCenter("Name cannot be empty. Aborting.\n");
        return;
    }

    printCenter("Enter age: ");
    if (scanf("%d", &p.age) != 1) {
        printCenter("Invalid age input.\n");
        while (getchar() != '\n'); // flush
        return;
    }
    while (getchar() != '\n'); // flush leftover newline

    printCenter("Enter disease: ");
    fgets(p.disease, sizeof(p.disease), stdin);
    p.disease[strcspn(p.disease, "\r\n")] = '\0';

    patients[patientCount++] = p;
    saveData();  /* autosave */
    printCenter("Patient added successfully!\n");
}

void viewPatients() {
    if (patientCount == 0) {
        printCenter("No patients registered.\n");
        return;
    }

    printCenter("\n--- Patient Records ---\n");
    for (int i = 0; i < patientCount; i++) {
        Patient *p = &patients[i];
        printf("%40sID: %d | Name: %s | Age: %d | Disease: %s\n",
               "", p->id, p->name, p->age, p->disease);
    }
}

void editPatient() {
    int id;
    printCenter("Enter Patient ID to edit: ");
    if (scanf("%d", &id) != 1) {
        printCenter("Invalid input.\n");
        while (getchar() != '\n'); // flush
        return;
    }
    while (getchar() != '\n');

    if (id < 1 || id > patientCount) {
        printCenter("Invalid Patient ID.\n");
        return;
    }

    Patient *p = &patients[id - 1];
    printCenter("Editing Patient Data\n");

    printCenter("Enter new name: ");
    fgets(p->name, sizeof(p->name), stdin);
    p->name[strcspn(p->name, "\r\n")] = '\0';

    printCenter("Enter new age: ");
    if (scanf("%d", &p->age) != 1) {
        printCenter("Invalid age input. Aborting edit.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    printCenter("Enter new disease: ");
    fgets(p->disease, sizeof(p->disease), stdin);
    p->disease[strcspn(p->disease, "\r\n")] = '\0';

    saveData(); /* autosave */
    printCenter("Patient record updated!\n");
}

void deletePatient() {
    int id;
    printCenter("Enter Patient ID to delete: ");
    if (scanf("%d", &id) != 1) {
        printCenter("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    if (id < 1 || id > patientCount) {
        printCenter("Invalid Patient ID.\n");
        return;
    }

    for (int i = id - 1; i < patientCount - 1; i++) {
        patients[i] = patients[i + 1];
        patients[i].id = i + 1;
    }

    patientCount--;
    saveData(); /* autosave */
    printCenter("Patient record deleted!\n");
}

void searchPatient() {
    int choice;
    char keyword[200];

    printCenter("\nSearch Patient By:\n");
    printCenter("1. Name\n");
    printCenter("2. Disease\n");
    printCenter("Enter choice: ");
    if (scanf("%d", &choice) != 1) {
        printCenter("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    printCenter("Enter keyword: ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\r\n")] = '\0';

    int found = 0;
    for (int i = 0; i < patientCount; i++) {
        if ((choice == 1 && strstr(patients[i].name, keyword)) ||
            (choice == 2 && strstr(patients[i].disease, keyword))) {
            printf("%40sID: %d | Name: %s | Age: %d | Disease: %s\n",
                   "", patients[i].id, patients[i].name, patients[i].age, patients[i].disease);
            found = 1;
        }
    }

    if (!found) printCenter("No matching patient found.\n");
}

/* ------------------ DOCTOR FUNCTIONS ------------------ */

void addDoctor() {
    if (doctorCount >= MAX_DOCTORS) {
        printCenter("Doctor list is full.\n");
        return;
    }
    Doctor d;
    d.id = doctorCount + 1;

    printCenter("Enter doctor name: ");
    fgets(d.name, sizeof(d.name), stdin);
    d.name[strcspn(d.name, "\r\n")] = '\0';

    printCenter("Enter specialty: ");
    fgets(d.specialty, sizeof(d.specialty), stdin);
    d.specialty[strcspn(d.specialty, "\r\n")] = '\0';

    doctors[doctorCount++] = d;
    saveData(); /* autosave */
    printCenter("Doctor added successfully!\n");
}

void viewDoctors() {
    if (doctorCount == 0) {
        printCenter("No doctors registered.\n");
        return;
    }

    printCenter("\n--- Doctor Records ---\n");
    for (int i = 0; i < doctorCount; i++) {
        Doctor *d = &doctors[i];
        printf("%40sID: %d | Name: %s | Specialty: %s\n",
               "", d->id, d->name, d->specialty);
    }
}

void editDoctor() {
    int id;
    printCenter("Enter Doctor ID to edit: ");
    if (scanf("%d", &id) != 1) {
        printCenter("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    if (id < 1 || id > doctorCount) {
        printCenter("Invalid Doctor ID.\n");
        return;
    }

    Doctor *d = &doctors[id - 1];
    printCenter("Editing Doctor Data\n");

    printCenter("Enter new name: ");
    fgets(d->name, sizeof(d->name), stdin);
    d->name[strcspn(d->name, "\r\n")] = '\0';

    printCenter("Enter new specialty: ");
    fgets(d->specialty, sizeof(d->specialty), stdin);
    d->specialty[strcspn(d->specialty, "\r\n")] = '\0';

    saveData(); /* autosave */
    printCenter("Doctor record updated!\n");
}

void deleteDoctor() {
    int id;
    printCenter("Enter Doctor ID to delete: ");
    if (scanf("%d", &id) != 1) {
        printCenter("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    if (id < 1 || id > doctorCount) {
        printCenter("Invalid Doctor ID.\n");
        return;
    }

    for (int i = id - 1; i < doctorCount - 1; i++) {
        doctors[i] = doctors[i + 1];
        doctors[i].id = i + 1;
    }

    doctorCount--;
    saveData(); /* autosave */
    printCenter("Doctor record deleted!\n");
}

void searchDoctor() {
    int choice;
    char keyword[200];

    printCenter("\nSearch Doctor By:\n");
    printCenter("1. Name\n");
    printCenter("2. Specialty\n");
    printCenter("Enter choice: ");
    if (scanf("%d", &choice) != 1) {
        printCenter("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    printCenter("Enter keyword: ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\r\n")] = '\0';

    int found = 0;
    for (int i = 0; i < doctorCount; i++) {
        if ((choice == 1 && strstr(doctors[i].name, keyword)) ||
            (choice == 2 && strstr(doctors[i].specialty, keyword))) {
            printf("%40sID: %d | Name: %s | Specialty: %s\n",
                   "", doctors[i].id, doctors[i].name, doctors[i].specialty);
            found = 1;
        }
    }

    if (!found) printCenter("No matching doctor found.\n");
}

/* ------------------ MAIN MENU ------------------ */

void mainMenu() {
    int choice;
    do {
        system("cls");

        printCenter("+----------------------------------------+\n");
        printCenter("|     HOSPITAL MANAGEMENT SYSTEM         |\n");
        printCenter("+----------------------------------------+\n\n");

        printCenter("1.  Add Patient\n");
        printCenter("  2.  View Patients\n");
        printCenter(" 3.  Edit Patient\n");
        printCenter("   4.  Delete Patient\n");
        printCenter("   5.  Search Patient\n");
        printCenter("6.  Add Doctor\n");
        printCenter(" 7.  View Doctors\n");
        printCenter("8.  Edit Doctor\n");
        printCenter("  9.  Delete Doctor\n");
        printCenter("10. Search Doctor\n");
        printCenter("11. Exit\n\n");

        printCenter("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            choice = -1;
            while (getchar() != '\n');
        }
        while (getchar() != '\n');

        switch (choice) {
            case 1: addPatient(); break;
            case 2: viewPatients(); break;
            case 3: editPatient(); break;
            case 4: deletePatient(); break;
            case 5: searchPatient(); break;

            /* Doctor operations:
               Add/Edit/Delete => require authentication
               View/Search => no authentication
            */
            case 6:
                if (authenticateDoctor()) addDoctor();
                break;
            case 7:
                viewDoctors();   /* no password */
                break;
            case 8:
                if (authenticateDoctor()) editDoctor();
                break;
            case 9:
                if (authenticateDoctor()) deleteDoctor();
                break;
            case 10:
                searchDoctor();  /* no password */
                break;

            case 11:
                printCenter("Exiting program...\n");
                break;
            default:
                printCenter("Invalid choice. Try again.\n");
        }

        printCenter("\nPress any key to continue...");
        _getch();

    } while (choice != 11);
}

/* Console center helper (keeps from being unused) */
void centerText(const char *text) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int width;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    int len = (int)strlen(text);
    int spaces = (width - len) / 2;

    for (int i = 0; i < spaces; i++) printf(" ");
    printf("%s\n", text);
}

int main() {
    /* Ensure data folder exists */
    CreateDirectoryA(DATA_FOLDER, NULL);

    /* Set console to UTF-8 so user input/output is UTF-8 */
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    /* IMPORTANT: The standard console font must support the characters you type.
       In many Windows consoles, "Consolas" or "Lucida Console" will work for most languages.
    */

    loadData();        /* load existing data (if any) */
    mainMenu();
    saveData();        /* final save on exit */
    return 0;
}
