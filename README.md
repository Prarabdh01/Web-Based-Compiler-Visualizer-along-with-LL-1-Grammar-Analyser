This project implements a web-based compiler phases visualizer for a small C-like language, combined with an LL(1) 
grammar analyzer. The main goal is to make the theoretical phases of compiler design—lexical analysis, syntax analysis, 
semantic analysis, intermediate code generation and optimization—easier to understand through an interactive browser 
interface. Users can type source code, run compilation, and instantly see the corresponding tokens, parse tree, symbol 
table, three-address code and optimized three-address code. This exposes the internal steps that full compilers usually hide. 
In parallel, the LL(1) grammar analyzer module allows students to experiment with context-free grammars. It presents 
grammar rules, FIRST and FOLLOW sets and an LL(1) parsing table for a standard expression grammar, illustrating how 
predictive parsing works and what conditions make a grammar LL(1). 
The system follows a three-tier architecture: a single-page HTML/CSS/JavaScript frontend, a Python Flask backend, and a 
C++ compiler core. The browser communicates with Flask using JSON over REST APIs; Flask forwards the source program to 
the C++ executable, which runs all compiler phases and returns results in machine-independent form. This modular design 
closely mirrors real-world compiler frontends while remaining compact enough for teaching. 
Ultimately, the project aims to bridge the gap between textbook theory and practical behaviour of compilers. It can be used 
for classroom demonstrations, lab exercises and self-study, giving students a clear visualization of how code is transformed 
from text into structured representations and optimized intermediate code. 
