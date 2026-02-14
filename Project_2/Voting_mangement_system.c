#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 10

struct Candidate {
    int id;
    char name[50];
    int votes;
};

struct Candidate candidates[MAX];
int candidateCount = 0;

// Function to add candidate
void addCandidate() {
    if (candidateCount >= MAX) {
        printf("Maximum candidates reached!\n");
        return;
    }

    printf("Enter Candidate ID: ");
    scanf("%d", &candidates[candidateCount].id);

    printf("Enter Candidate Name: ");
    scanf("%s", candidates[candidateCount].name);

    candidates[candidateCount].votes = 0;
    candidateCount++;

    printf("Candidate added successfully!\n");
}

// Function to display candidates
void displayCandidates() {
    if (candidateCount == 0) {
        printf("No candidates available!\n");
        return;
    }

    printf("\nList of Candidates:\n");
    for (int i = 0; i < candidateCount; i++) {
        printf("ID: %d | Name: %s\n",
               candidates[i].id,
               candidates[i].name);
    }
}

// Function to cast vote
void castVote() {
    int id, found = 0;

    if (candidateCount == 0) {
        printf("No candidates available!\n");
        return;
    }

    displayCandidates();
    printf("Enter Candidate ID to vote: ");
    scanf("%d", &id);

    for (int i = 0; i < candidateCount; i++) {
        if (candidates[i].id == id) {
            candidates[i].votes++;
            printf("Vote cast successfully!\n");
            found = 1;
            break;
        }
    }

    if (!found)
        printf("Invalid Candidate ID!\n");
}

// Function to show results
void showResults() {
    if (candidateCount == 0) {
        printf("No candidates available!\n");
        return;
    }

    printf("\nVoting Results:\n");
    for (int i = 0; i < candidateCount; i++) {
        printf("Name: %s | Votes: %d\n",
               candidates[i].name,
               candidates[i].votes);
    }

    // Find winner
    int maxVotes = 0, winnerIndex = 0;
    for (int i = 0; i < candidateCount; i++) {
        if (candidates[i].votes > maxVotes) {
            maxVotes = candidates[i].votes;
            winnerIndex = i;
        }
    }

    printf("\nWinner: %s with %d votes\n",
           candidates[winnerIndex].name,
           maxVotes);
}

int main() {
    int choice;

    while (1) {
        printf("\n===== Voting Management System =====\n");
        printf("1. Add Candidate\n");
        printf("2. Display Candidates\n");
        printf("3. Cast Vote\n");
        printf("4. Show Results\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addCandidate();
                break;
            case 2:
                displayCandidates();
                break;
            case 3:
                castVote();
                break;
            case 4:
                showResults();
                break;
            case 5:
                exit(0);
            default:
                printf("Invalid choice!\n");
        }
    }

    return 0;
}
