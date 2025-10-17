# Secure ATM Protocol System

## A Secure Client–Server Banking Simulation in C

This project implements a secure ATM–Bank communication system in C using UDP sockets and OpenSSL for encryption.  
It simulates the workflow between an ATM client, a Bank server, and a Router that passes messages between them.

---

## Introduction

The **Secure ATM Protocol System** is a C-based client–server project that simulates encrypted communication between an ATM terminal, a bank server, and a network router.  
It uses **UDP sockets** for message passing and **OpenSSL’s `libcrypto`** library for encryption, authentication, and secure session management.  
Originally developed as part of a **Computer Systems Security** course at the **University of Maryland**, this project demonstrates key concepts in **network programming**, **secure protocols**, and **buffer-overflow protection**.  

---

## How to Run the Project

### 1: Build the System
From the project root directory:
```bash
make clean
make all
```
This compiles all components into the `/bin` folder:
```
bin/atm      # ATM client
bin/bank     # Bank server
bin/router   # Message router
bin/init     # Initialization utility
```

### 2: Initialize System Files
Before running, create the initial state files for the ATM and Bank:
```
./bin/init bank
./bin/init atm
```
These commands generate:
```
atm.atm  atm.bank
bank.atm bank.bank
```

### 3: Launch Components (in separate terminals)
Terminal 1 (Router):
`./bin/router`

Terminal 2 (Bank):
`./bin/bank bank.bank`

Terminal 3 (ATM):
`./bin/atm atm.atm`

---


## Commands

### ATM Commands

The ATM supports a series of user commands entered at the terminal prompt. 
Each command should be typed out after the prompt (`ATM:`).

#### begin-session

Starts a new session for the specified user.
```bash
begin-session <user-name>
```

**Behavior:**
- If a user is already logged in:
   `A user is already logged in`
- If `<user-name>` is invalid (non-alphabetic or too long):
   `Usage: begin-session <user-name>`
- If user does not exist:
   `No such user`
- If `<user-name>.card` cannot be accessed:
   `Unable to access <user-name>'s card`
- If the PIN entered is incorrect:
   `Not authorized`
- If the PIN is correct:
   `Authorized`
   The prompt changes too:
   `ATM (<user-name>):`


#### withdraw 

Withdraws a specified amount of money from the logged-in user’s account.

```bash
withdraw <amt>
```

**Behavior:**

- If no user is logged in:
   `No user logged in`

- If input is invalid (non-integer or negative):
   `Usage: withdraw <amt>`

- If the user has insufficient funds:
   `Insufficient funds`

- If successful:
   `$<amt> dispensed`

#### balance

Displays the logged-in user’s current account balance.

```bash
balance
```

**Behavior:**

- If no user is logged in:
   `No user logged in`

- If input is invalid:
   `Usage: balance`

- Otherwise:
   `$<balance>`

#### end-session

Logs out the current user and ends the session.
```bash
end-session
```

**Behavior:**

- If no user is logged in:
   `No user logged in`

- Otherwise:
   `User logged out`
   The prompt returns to:
   `ATM:`

#### Invalid Commands

Any unsupported command (e.g., `deposit`, `transfer`, or typos) will result in:

`Invalid command`

Example ATM Sessions
```bash
ATM: begin-session Aliceeee
No such user

ATM: begin-session Alice
PIN? 1234
Authorized

ATM (Alice): balance
$100

ATM (Alice): withdraw 1
$1 dispensed

ATM (Alice): balance
$99

ATM (Alice): end-session
User logged out

ATM: balance
No user logged in
```

---

### Bank Commands

The Bank program manages user accounts and handles requests from both the operator and the ATM system.  
Once running, the prompt will appear as:
`BANK:  `

#### create-user

Creates a new user account and generates a corresponding `.card` file for that user.

```bash
create-user <user-name> <pin> <balance>
```

**Behavior**
- If inputs are invalid (bad format or missing arguments):
   `Usage: create-user <user-name> <pin> <balance>`

- If the user already exists:
   `Error: user <user-name> already exists`

- If the card file cannot be created:
   `Error creating card file for user <user-name>`

- If successful:
   `Created user <user-name>`

**Notes:**
- Usernames can only contain letters (`[a-zA-Z]+`) and must be ≤ 250 characters.

- PINs must be 4 digits (`[0-9][0-9][0-9][0-9]`).

- Initial balance must be a non-negative integer.

- Card files cannot be modified after creation.

#### deposit

Adds funds to a user’s account.

```bash
deposit <user-name> <amt>
```

**Behavior:**

- If inputs are invalid:
   `Usage: deposit <user-name> <amt>`

- If no such user exists:
   `No such user`

- If the deposit would overflow the integer limit:
   `Too rich for this program`

- If successful:
   `$<amt> added to <user-name>'s account`

#### balance

Displays the current balance of a user.

```bash
balance <user-name>
```

**Behavior:**

- If inputs are invalid:
   `Usage: balance <user-name>`

- If no such user exists:
   `No such user`

- Otherwise:
   `$<balance>`

#### Invalid Comments

Any unsupported or mistyped command will result in:

```bash
Invalid command
```

Withdrawals and user deletions are not supported at the Bank level.

Example Bank Session:
```bash
BANK: balance Alice
No such user

BANK: create-user Alice 1234 100
Created user Alice

BANK: deposit Alice 2
$2 added to Alice's account

BANK: balance Alice
$102
```

---


## Author
**Felix Gu**
[LinkedIn](https://www.linkedin.com/in/felix-gu/) • [GitHub](https://github.com/f-gu12)