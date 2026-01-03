# 4th Semester Challenge - AGV and ScissorLift
This repository contains my contributions to the project, along with the final technical report submitted to the supervising professors, which documents the design, implementation, and results of the work.

The project lasted 15 weeks:
- Weeks 1–5: Project planning and static analysis of the Scissor Lift.
- Weeks 6–10: CAD design and manufacturing of the Scissor Lift and AGV.
- Weeks 11–15: Electronics schematic, connections, and programming.

---

## Repository Structure
├── Programming/

│   ├── AGV_State_Machine/         → AGV state machine logic

│   ├── ScissorLift_StateMachine/  → Scissor Lift state machine logic

│   └── Tests/

│       ├── AGV_tests/           → Individual AGV component tests

│       └── Scissor_Lift_tests/  → Individual Scissor Lift component tests

├── Static_Analysis/             → Force calculations and dimension estimations

├── Manufacturing_Photos/        → Visual documentation of fabrication process

├── AGV_SL - Final Report.pdf    → Final technical report

├── LICENSE

└── README.md

---

## Weeks 1 - 5: Project Planning and Static Analysis
My contributions focused on static analysis calculations. Two members performed independent analyses to compare results. Here I present my formulation.

### Force and Moment Diagrams
![Forces and momtums acting per link](./Static_Analysis/Whiteboard_1.jpeg)
![Equations to relate the variables, stress used, and Minimum Case analysis](./Static_Analysis/Whiteboard_2.jpeg)

### Digitalized Diagrams and Matrix Formulation
![Force diagrams per link](./Static_Analysis/Digital_1.jpeg)
![Equations used, variables, and constants](./Static_Analysis/Digital_2.jpeg)
![Matrix used and Net Force per joint](./Static_Analysis/Digital_3.jpeg)

### Results
- Internal forces per link were estimated using the matrix-based solution.
- A preliminary stress-based sizing approach was explored using shear stress assumptions.
- These results were used for qualitative comparison between different modeling approaches.

![Links 2, 3, and 4 minimum height calculation](./Static_Analysis/Final_1.jpeg)
![Links 5, 6, and 7 minimum height calculation](./Static_Analysis/Final_2.jpeg)

### Lessons Learned

- **Matrix Formatulation**:
  - *Initial issue:* known loads and unknown forces were not consistently separated in the equilibrium matrix.
  - *Correction implemented:* reformulated the system by isolating unknowns in the coefficient matrix (A) and known loads in the load vector (b) *(Content to be added)*.
  - *Engineering lesson:* accurate separation of known and unknown terms is critical in matrix-based static models.

- **Stress Evaluation**
  - *Initial limitation:* Link sizing was based solely on shear stress assumptions.
  - *Planned refinement:** extend the stress analysis to include additional failure modes relevant to scissor lift mechanisms *(Content to be added)*:
	- *Buckling:* instability of slender members under compressive loads.
	- *Bearing:* localized crushing at pin and plate interfaces.
	- *Yielding:* material yielding under axial and combined stresses.
	- *Fatigue:* potential crack initiation under repeated load cycles.
  - *Engineering lesson:* mechanical design requires evaluating multiple failure modes to ensure structural safety and reliability.

### Corrections
*(Content to be added)*

---

## Weeks 6–10: CAD and Manufacturing 
The manufacturing methods we applied during the course were:
- **Sawing:** Mainly for the Scissor Lift, cutting pieces for the body. 
![Sawing Example](Manufacturing_Photos/Sawing.jpeg)
- **Sheet Metal Working:** Mainly for the AGV.

![Sheet Metalworking Example](Manufacturing_Photos/SheetMetalworking.jpeg)
- **Drilling:** Used in both systems to make holes for screws and rivets.
![Drilling Example](Manufacturing_Photos/Drilling.jpeg)
- **Welding:** Mainly in the Scissor Lift, to join the body pieces together.
![Welding Example](Manufacturing_Photos/Welding.jpg)
- **Milling:** Used in the Scissor Lift, enabling lifting and tilting.
![Milling Example 1](Manufacturing_Photos/Milling.jpeg)
![Milling Example 2](Manufacturing_Photos/Milling_2.jpeg)
- **Lathe:** Initially used to achieve exact component sizes, but later we realized it was more efficient to buy standard measurements.

### Results 
The static analysis helped determine the minimum and optimal dimensions for the Scissor Lift. We differentiated from other teams in: 
- **Weight:** Most other teams built Scissor Lifts weighing more than 20 kg, while ours weighed less than 6 kg. 
![Our Scissor Lift's weight](Manufacturing_Photos/Peso.jpeg) 
- **Dimensions:** Other teams’ Scissor Lifts were oversized for the intended load. By performing static analysis, we calculated the optimal dimensions required. 
![Results photo](Manufacturing_Photos/Result.jpeg) 
![Scissor Lift carrying the intended load](Manufacturing_Photos/Result_2.jpeg)

### Lessons Learned 
- **Overchecking dimensions:** 
  - Due to a lack of manufacturing experience, we assumed it would be easy to achieve exact dimensions to avoid material waste. In practice, it was time-consuming and difficult, resulting in broken pieces and rework.
  - It was also challenging to find already-manufactured parts (like bearings) matching our calculated sizes. We learned that using **standard components** is often more practical and reliable.

---

## Weeks 11–15: Electronics and Programming

During the final weeks of the project, I was responsible for coding the full programming of both the AGV and the Scissor Lift. This was because I was the only team member with prior experience in the C language from another project. Meanwhile, my teammates focused on the electronics design, schematics, and wiring of both subsystems.

### AGV State Machine
*(Add AGV diagram here)*

### Scissor Lift State Machine
*(Add Scissor Lift diagram here)*

Before writing the final state machine code, I first created diagrams of the professor-provided libraries to fully understand their functionality. Then, I developed **component-level test programs** for both the AGV and the Scissor Lift (available in the `Tests` folder). Finally, I implemented the complete state machine logic for each subsystem (available in their respective folders).

> **Note:** This repository only contains my implementation (`src/main.cpp`) and configuration (`src/definitions.h`). External libraries provided by the professor are not included due to licensing. To run the project, please add the required libraries manually in the `/lib` folder.

### Results
Although the final integration of the AGV and Scissor Lift did not succeed, I documented and attached videos of the individual component tests. These demonstrate that each subsystem worked correctly in isolation.  
*(Attach videos of the component tests here)*

### Lessons Learned

- **Time Management**
  - On the day of the presentation, we were still wiring the AGV and Scissor Lift in the morning. Just five minutes before presenting, the system stopped working, and we were frantically rewiring and checking connections.
  - The main issue was that we did not allocate time for **integration testing**. While component tests worked perfectly, we never tested the final state machine algorithm as a whole.
  - **Correction implemented:** In future projects, I will reserve at least one week exclusively for integration and final testing to ensure the complete system works reliably before presentation.

- **Wiring Issues**
  - As this was our first mechatronics project, most of our coursework had been introductory. We did not yet know how to design and manufacture a PCB, which would have prevented excessive wiring, electrical noise, and loose connections.
  - We relied on Dupont cables and protoboards, which are useful for prototyping but unreliable for final integration. While all component tests worked individually, once assembled, cables disconnected easily or signals were lost due to poor connections.
  - **Engineering lesson:** I learned the importance of PCB design and proper wiring practices. In future projects, I will use manufactured PCBs and standard gauge cables (e.g., AWG 22) instead of Dupont cables, ensuring robust and reliable connections for final integration.

---

## Final Reflections

This was my first real-world project and the first time I was actively responsible for multiple areas of development. In previous projects (e.g., Blue Car Project, Shell Eco Marathon), I mainly participated as an assistant or focused on learning from other members. This time, I had to take ownership and lead significant parts of the work.

We transitioned from purely theoretical courses (Math, Physics, etc.) to more career-relevant, though still introductory, subjects such as Manufacturing, AC Circuits, Basics of Microcontrollers, and Project Management. Considering our limited background, it was expected that we would face difficulties. Fortunately, most subsystems worked as intended, and the only major failure was during the final integration, which, as several professors noted, is often the hardest stage to perfect.

Although I am not fully satisfied with the final implementation, I am proud of the learning process and the fact that I applied new knowledge directly to this project. As a team, we need to improve time management, continue strengthening the skills we developed, and expand into areas not yet covered by our courses — such as PCB design. This project also taught me the importance of thorough testing and dedicating time specifically for integration. These lessons will continue to shape my growth in the coming semesters and years.

### Key Takeaways
- **Mechanics:** Learned about static analysis, CAD design, manufacturing processes, and how to select mechanical components (e.g., bearings) using engineering handbooks.  
- **Electronics:** Gained experience with AC circuits and basic applications of diodes, transistors, and operational amplifiers.  
- **Programming:** Learned to structure C++ code, implement state machines, and control multiple actuators and sensors in real time using microcontrollers like the ESP32.  
- **Project Management:** Practiced risk management analysis and developed Gantt charts to plan and track progress.  

### Conclusion
Even though the final prototype faced challenges, this project was a milestone in my engineering journey. It gave me my first true experience leading a mechatronics project, taught me the importance of theory before practice, and highlighted the value of documentation and testing. Most importantly, it gave me the confidence to tackle complex systems and reinforced my commitment to building reproducible, well-documented, and ethical engineering solutions.
