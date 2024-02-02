# Multithreaded Text Search Tool (MTST)

The **Multithreaded Text Search Tool (MTST)** is a command-line utility inspired by `grep` in Linux. It allows users to search for a specified pattern within text files in a given directory and its subdirectories. MTST leverages multithreading, process creation, synchronization mechanisms, and interprocess communication to efficiently perform the search.

## Features

1. **Search Text Files**: MTST recursively scans all `.txt` files in the specified directory and its subdirectories to find occurrences of the user-provided pattern.

2. **Multithreading and Process Creation**:
   - For each subdirectory encountered during traversal, MTST creates a new process to handle the search within that subdirectory.
   - Within each process, multiple threads are spawned to search individual text files concurrently.

3. **Synchronization Mechanisms**:
   - MTST uses semaphores and locks to ensure proper synchronization between processes and threads.
   - Semaphores prevent race conditions when accessing shared resources (e.g., counting matches).
   - Locks protect critical sections (e.g., updating match counts).

4. **Output Format**:
   - The program outputs results in the following format:
     ```
     ./directory/filename.txt:lineNumber:characterNumber:ID:time
     ```
     - `lineNumber`: The line where the pattern was found.
     - `characterNumber`: The position of the first matching character in that line.
     - `ID`: The thread/process ID responsible for the search.
     - `time`: The execution time of the thread/process.

5. **User Interface (UI)**:
   - The UI is built using **Electron.js**, providing an intuitive interface for users to input the directory and pattern.
   - To set up the UI, follow the installation instructions below.

## Installation

1. Clone this repository:
 
2. Install dependencies:
   ```
   npm install
   ```

## Usage

1. Open the terminal and navigate to the project directory.

2. Run the program:
   ```
   npm run start
   ```

3. In the UI, enter the following:
   - **Directory Path**: The absolute path to the directory containing the text files.
   - **Pattern**: The search pattern (plain text).

4. Click the "Search" button.



## Notes

- Adjust the number of threads and processes based on system resources and performance requirements.
- Ensure proper error handling for file I/O, directory permissions, and other edge cases.

Feel free to enhance and customize MTST according to your needs. Happy coding! 🚀