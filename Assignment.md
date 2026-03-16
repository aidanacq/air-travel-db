# CIS 22C Honor Cohort: Capstone Project

**Converted from original DOCX file.** fileciteturn0file0

## Create a front-end website for an Air Travel Database

### I. Overview

This Honor’s Cohort project focuses on learning a (potentially) new skill which involves major collaboration with an AI tool. While knowledge (such as understanding how to code in C++ and how a web application works) is useful, it is even more valuable when it is incorporated into a skill that you can do something with it (such as actually coding and deploying your own C++ web application).

In essence, through an ongoing chat session with an AI tool like ChatGPT (which might require many interactions at each level below), you will:

1. **Describe** the functionality of an Air Travel Database web application (based on the information contained further down in this project description).
2. **Work with the AI tool** until all the code implementing the web application is generated. The code must be in C++.
   - Note: C++ v2003 is the default version used in class examples, but later versions are acceptable.
3. **Get the AI tool** to use its generated C++ code to build, link and create the web application successfully.
4. **Access** the web application locally with your browser via port 8080 and test it to ensure it works.
   - Note: If you’ve never “Vibe Coded” before, the first time you succeed in doing this is the moment when you realize the power of AI in converting your application visions into actual working code.
5. **Deploy** your web application so that others (for example, the instructor) can access it. This can be done in one of two ways:
   - Deployed locally but accessible externally.
   - Deployed remotely on the web.

The remainder of this document describes the purpose and specifics of the web application you are being tasked with creating and deploying. You will almost certainly have a lot of questions you need answered before you can complete that task, but the only questions you can ask the instructor are to clarify what they want the application to do.

For answers to any questions about “how do I do this”, or “why did I get this error”, you will need to ask the AI tool you are using.

#### Upon Completion

Once you have completed this project to your satisfaction, you will be assigned a slot in one of the instructor’s “extra” office hours to present your web application, show off its features, detail the use of Data Structures and Data Abstractions and answer a few questions about what you learned in its construction.

This presentation and the work behind it will determine **20%** of your grade in this course. It is possible to earn significant extra credit for incorporating additional functionality into the web application you present and describe.

---

### II. Creating a front-end website for the “Openflights” Air Travel DB

#### 1. Data Source

**www.openflights.org/data.php**

This website provides copyright-free CSV “.dat” files for the entities:

- Airlines  
- Airports  
- Directional Routes

and a field-by-field explanation of the data comprising each entity. Your first challenge will be figuring out how to get that information into your AI tool.

#### 2. Data File Summary

- All the Airlines data entries are self-contained. Each entity includes an internal unique OpenFlights Airline ID integer which the Routes use to reference it (the Airline “Index”). Site users never see this — they use a unique Airline IATA Code to identify the Airline (ex: `AA` for American Airlines).

- All the Airports data entries are also self-contained, and each includes an internal unique OpenFlights Airport ID integer (the Airport “Index”) which the Routes use to reference it. Site users never see this — they use a unique Airport IATA Code to identify the Airport (ex: `SFO` for San Francisco Airport).

- Each Routes data entity is directional. It connects one “Source” Airport to one “Destination” Airport and indicates the Airline which flies that Route. Both the Airports and the Airline are identified in the Route entity through their OpenFlights IDs.

#### 3. Assignment Summary

There are **4 parts** to this Capstone Project.

##### A. Logistics

The ultimate goal of this section requires creating an externally hosted web application which reflects the C++ Vibe Coding work you have done, and which is simultaneously accessible by multiple browser-based clients.

There are three possible deployment options:

1. **Use Port 8080** on your system for internal access to your Web Application  
   - Good for internal debugging. Not viable as a final solution for external access. This option will receive less than full credit.

2. **Make Port 8080 Externally Accessible**  
   - Running a utility (ex: `ngrok` for Windows) can make your Port 8080 externally accessible. This option receives full credit (requires coordination).

3. **Deploy your project software on the web**  
   - Options: `fly.io`, Heroku, or major cloud providers. This option receives extra credit when reachable at a fixed URL.

**Notes:**
- Option 3 may require installing additional free software and handling platform differences (e.g., case sensitivity).
- You may generate the application in stages — demonstrate some level of web application functionality achieved without writing a line of code.

---

### III. Basic Functionality

The following data operations must be available to all users via their browser front end. Whenever an Airport or Airline entity is being returned, all of its elements must be provided.

#### 1. Individual Entity Retrieval

- **1.1** Given an OpenFlights Airline IATA Code, return the corresponding Airline entity.
- **1.2** Given an OpenFlights Airport IATA Code, return the corresponding Airport entity.

#### 2. Airline and Airport Reports

_All Reports are typically in the form of a CSV file or (better) dynamically displayed in a response box._

**2.1 Reports Ordered by # Routes**

a. **Given an Airline IATA Code:**  
Return the selected Airline Name and all Airports reached by the Airline’s Routes ordered by total # Airline Routes to/from that Airport. Each Airport appears once and includes an extra element `# routes`.

b. **Given an Airport IATA Code:**  
Return the selected Airport Name and all Airlines with Routes that include that Airport ordered by total # Routes flown by each Airline to/from that Airport. Each Airline appears once and includes an extra element `# routes`.

**2.2 Reports Ordered by IATA Codes**

- a. Airline Report: Return all Airline entries ordered by Airline IATA Code.  
- b. Airport Report: Return all Airport entries ordered by Airport IATA Code.

**2.3 Get ID**

- Returns your De Anza Student ID # and Name.

> Note: Getting this far should be fairly straightforward. If you stopped here, your grade on the capstone project would be a C.

---

### IV. Additional Functionality

Completing any of the following extensions will raise your Capstone Project one grade. Completing more than 2 of them can earn you extra credit.

#### 1. Updating OpenFlights Data

Updating the OpenFlights data is done by inserting / deleting / modifying Airline, Airport and / or Route entries, subject to the legality checks contained in the table below.

| Operation | Airport Req | Airline Req | Route Req |
| --- | --- | --- | --- |
| **Insert** | Airport ID is unique; IATA code is unique | Airline ID is unique; IATA code is unique | Airports & Airline IDs are valid |
| **Modify** | ID / IATA is unchanged; any field not specified is unchanged | ID / IATA is unchanged; any field not specified is unchanged | Airport/Airline IDs are valid (unique) if changed; any field not specified is unchanged |
| **Remove** | Remove Airport entity with specified IATA & remove all Routes to/from that Airport | Remove Airline entity with specified IATA & remove all Routes of that Airline | Route entity with specified Source / Destination Airport IDs & Airline ID is removed |

> Important: It is perfectly acceptable (and easier) to make these changes only to in-memory managed data and not to the actual on-disk storage. If you do this, every time the web application is restarted, the initial data is the same. If you remotely host your web application, this option also minimizes hosting costs; just indicate whether you’ve done this.

#### 2. Get Code

- Returns a viewable file of all the Vibe-generated C++ code implementing this website.

#### 3. “One hop” Report capability

Given the IATA codes of both a Source `S` and Destination `D` Airport:

- Find all Route “pairs” (`S -> X` / `X -> D`) where both Routes have 0 stops (so the ultimate route is 1 hop).
- For every Route pair found, use the Airport GPS to calculate total route air miles and then order the Route pairs in the report.
- Displaying a map with the top 5 intermediate cities in terms of # of routes is extra credit.

#### 4. Enhanced Look & Feel

- Improve look and functionality: photos, charts, buttons, subpages (About / Contact), animation, displaying routes on a world map, etc.

---

### V. Documenting the Usage of Collection Class Data Structures

For each of the 3 Entities (Airlines, Airports, Routes) in your solution, submit a brief written description.

1. **What are the C++ Data Structure(s)** that collect the basic Entities:
   - Airlines
   - Airports
   - Routes

2. **How are Routes linked** to an Airline and the Start / End Airports?
   - Numeric IDs
   - Smart Pointers

3. **Describe the algorithm** that converts input to output:

- **Input:** An IATA Airline code `XX`  
- **Output:** The complete Airline Report, listing the IATA Codes and names of all Airports touched by an `XX` Route, ordered by the total number of `XX` Routes that include that Airport (which is also shown).

_Example: If `AA` is entered, the first 2 lines of the report might be:_

```
DFW    Dallas Fort Worth International    362
CLT    Charlotte Douglas International    268
```

4. **Change the assumptions:** Identify and concisely explain changes needed to your original design decisions (type of collections, types of linkages) for:
- The size of each Airline and Airport record increases 1000×
- The number of Airlines and Airports increases 1000×
- The number of Routes connecting Airlines to Airports increases 1000×
- The ratio of (Retrievals + Reports) / Updates changes from 100:1 to 1:100

---

*End of document.*
