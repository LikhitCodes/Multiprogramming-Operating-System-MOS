# Multiprogramming Operating System (MOS) Simulator

This project simulates a Multiprogramming Operating System (MOS) designed to execute programs by managing CPU resources, memory, and input/output operations. The project is implemented in progressive phases, starting with a basic single-job operating system and moving towards more complex features like paging and error handling.

---

## Phase 1: Basic Operating System Simulation

### 1. Introduction
In Phase 1, the system executes only one program at a time to focus on understanding the core working of an operating system. It simulates how an OS reads programs from an input file, loads them into memory, executes instructions step-by-step, and performs input/output operations using service interrupts.

### 2. Virtual Machine Model & Memory
- The system consists of 100 memory locations (`00` to `99`).
- Each memory location stores a word of 4 bytes (characters).
- Instructions (e.g., `GD10`, `LR20`) and Data are stored in these locations.
- Programs are loaded starting from memory location `00`.

### 3. CPU Registers
| Register | Size | Description |
|----------|------|-------------|
| **IR** (Instruction Register) | 4 bytes | Stores the currently executing instruction |
| **IC** (Instruction Counter) | 2 bytes | Points to the next instruction |
| **R** (General Purpose Register) | 4 bytes | Used for temporary storage |
| **C** (Toggle) | 1 byte | Condition flag used for comparison (`T` or `F`) |

### 4. Instruction Set
Each instruction is 4 characters (2 bytes Opcode, 2 bytes Operand):
- `LR xx`: Load register with memory value.
- `SR xx`: Store register value in memory.
- `CR xx`: Compare register with memory.
- `BT xx`: Branch to address if condition toggle `C` is True.
- `GD xx`: Read data into a memory block (Service Interrupt 1).
- `PD xx`: Write data from a memory block to output (Service Interrupt 2).
- `H`: Halt execution (Service Interrupt 3).

### 5. Interrupt Mechanism (Master Mode)
Phase 1 uses **Service Interrupts (SI)** for I/O and termination:
- `SI = 1`: READ operation (`GD`)
- `SI = 2`: WRITE operation (`PD`)
- `SI = 3`: TERMINATE (`H`)

### 6. Job Control Cards
The input file is structured using special control cards to define program boundaries:
- `$AMJ`: Indicates start of a job.
- `$DTA`: Indicates start of data.
- `$END`: Indicates end of a job.

---

## Phase 2: Paging and Error Handling

### 1. Introduction
Phase 2 introduces memory paging, multi-programming concepts, process control blocks, hardware interrupts for errors, and resource limits (time and lines). The system now simulates Virtual Address to Real Address mapping.

### 2. Assumptions & New Features
- Jobs may have program errors.
- **Paging introduced**: Page table is stored in real memory.
- Program pages are allocated to one of 30 memory blocks (frames) using a random number generator.
- Loads and runs one program at a time.
- **Time limit**, **Line limit**, and **Out-of-data** errors are introduced.
- **TI (Timer Interrupt)** and **PI (Program Interrupt)** introduced.
- Terminations include 2-line error messages.
- Job outputs are separated in the output file by 2 blank lines.

### 3. Additional Registers and Data Structures
- **PTR** (Page Table Register): 4 bytes.
- **PCB** (Process Control Block): Data structure containing job details.
- **VA** (Virtual Address) & **RA** (Real Address).
- **TTC** (Total Time Counter) & **TTL** (Total Time Limit).
- **LLC** (Line Limit Counter) & **TLL** (Total Line Limit).
- **EM** (Error Message): Indicates the termination status.

### 4. Interrupt Values
**Service Interrupt (SI):**
- `SI = 1`: READ (`GD`)
- `SI = 2`: WRITE (`PD`)
- `SI = 3`: TERMINATE (`H`)

**Program Interrupt (PI):**
- `PI = 1`: Operation Error
- `PI = 2`: Operand Error
- `PI = 3`: Page Fault

**Timer Interrupt (TI):**
- `TI = 2`: Time Limit Exceeded

### 5. Error Message Coding (EM)
| EM | Error |
|----|-------|
| 0 | No Error |
| 1 | Out of Data |
| 2 | Line Limit Exceeded |
| 3 | Time Limit Exceeded |
| 4 | Operation Code Error |
| 5 | Operand Error |
| 6 | Invalid Page Fault |

### 6. Interrupt Handling (Master Mode)
When interrupts occur, the MOS transitions to Master Mode and evaluates `TI`, `SI`, and `PI`.

#### TI and SI Combinations:
| TI | SI | Action |
|----|----|--------|
| 0 | 1 | READ |
| 0 | 2 | WRITE |
| 0 | 3 | TERMINATE (0) |
| 2 | 1 | TERMINATE (3) |
| 2 | 2 | WRITE, THEN TERMINATE (3) |
| 2 | 3 | TERMINATE (0) |

#### TI and PI Combinations:
| TI | PI | Action |
|----|----|--------|
| 0 | 1 | TERMINATE (4) |
| 0 | 2 | TERMINATE (5) |
| 0 | 3 | If Page Fault Valid: ALLOCATE, update page Table, Adjust IC if necessary, EXECUTE USER PROGRAM. OTHERWISE TERMINATE (6) |
| 2 | 1 | TERMINATE (3,4) |
| 2 | 2 | TERMINATE (3,5) |
| 2 | 3 | TERMINATE (3) |

### 7. Address Translation
The system uses an **ADDRESS MAP** function to convert Virtual Addresses (VA) to Real Addresses (RA).
- If an invalid page is accessed, a Page Fault (`PI = 3`) is generated.
- If the operand is invalid, an Operand Error (`PI = 2`) is generated.
- Valid page faults lead to frame allocation and page table updates.

### 8. System Operations
- **READ**: Checks if the next card is `$END` (Out of Data, `EM = 1`). Otherwise, reads a data card into memory and executes.
- **WRITE**: Increments LLC. If `LLC > TLL`, terminates with `EM = 2` (Line Limit). Otherwise, writes the block and executes.
- **TERMINATE**: Writes 2 blank lines and the corresponding error messages. Control is returned to OS to LOAD the next job.
