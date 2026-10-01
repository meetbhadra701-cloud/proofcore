# Milestone 1 Architecture

This note summarizes the frozen and implemented M1 architecture. The normative details live in `SPEC.md`; this diagram makes the instruction and control flow visible.

```mermaid
flowchart LR
  IMEM[Instruction memory<br/>combinational read] --> IF[IF<br/>PC / PC+4]
  IF --> IFID[(IF/ID<br/>valid + PC + instruction)]
  IFID --> ID[ID<br/>decode / regfile / immediate]
  RF[(32 x 32 register file<br/>2 reads / 1 write)] <--> ID
  ID --> IDEX[(ID/EX<br/>valid + operands + controls)]
  IDEX --> EX[EX<br/>forward / ALU / compare / target]
  EX --> EXMEM[(EX/MEM<br/>valid + result + memory controls)]
  EX -->|taken redirect| IF
  EX -->|flush younger| IFID
  EX -->|flush younger| IDEX
  EXMEM --> MEM[MEM<br/>load select / store lanes]
  DMEM[Data memory<br/>combinational read / edge write] <--> MEM
  MEM --> MEMWB[(MEM/WB<br/>valid + result + trace fields)]
  MEMWB --> WB[WB<br/>writeback / retirement]
  WB --> RF
  WB --> RET[Milestone-1 retirement record<br/>not RVFI]
  ID -->|load-use stall| IF
  ID -->|hold IF/ID, bubble ID/EX| IFID
```

`EX` is the only control-transfer decision point. An EX redirect changes the next fetch PC and clears younger IF/ID and ID/EX validity. A load-use condition holds the fetch PC and IF/ID for one cycle and injects an invalid ID/EX entry. If a redirect and stall appear together, redirect takes priority.

The two memory ports are separate and fixed latency. All architectural updates appear at WB, in order. The internal retirement record exposes that commit point without claiming RVFI semantics.
