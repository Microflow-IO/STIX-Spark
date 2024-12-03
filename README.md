<img src="https://github.com/Microflow-IO/microflow-nano/blob/main/docs/github_microflow_B.png" alt="logo" style="float:left; margin-right:10px;" />



# STIX Spark



## 🎬Background

There are already several excellent projects for managing and matching various STIX libraries, but why are we still developing STIX Spark? The reason is actually very simple: they are all too complex. 

Our only requirement is to use a vast amount of logs as source data, efficiently match various STIX libraries in real time, and send the matching results back to SOC or other log analysis platforms. However, after researching several well-known STIX projects on Github, I felt that none of them quite met this goal. So, We decided to write a program in two or three days to achieve this goal myself.



## 🛠Introduction

STIX Spark is a program that utilizes any type of log as source data, offering a simple and efficient approach to matching against the STIX librarys and concurrently returning alerts.

Spark is typically deployed behind Logstash/Fluentd, ELF/Splunk/Graylog, or other SOC systems. 

Together with Modsecurity for Anylog (MSA), another project of Microflow.io, Spark analyzes high-quality traffic logs output by Nano to detect attack risks in cloud and cloud-native environments, monitor sensitive data behaviors, and perform threat intelligence STIX matching. 

If Nano is also integrated with mainstream log analysis platforms, it can also detect network-level attack risks and risks such as the use of weak passwords.



**Threat Intelligence Matching**

```mermaid
graph TB  
    subgraph "Cloud and Cloud-Native Environment"  
        VM1["Virtual Machine 1 (with Nano)"]  
        VM2["Virtual Machine 2 (with Nano)"]  
        Node1["Node 1 (with Nano Pod)"]  
        Node2["Node 2 (with Nano Pod)"]  
    end  

    Middleware["Logstash/Fluentd"]  
    MSA["STIX Spark"]
    MSA<-.->L1[Threat Intelligence Library]
    MSA<-.->L2[Mining Address Library]    
    AnalyticsPlatform["Data Analytics Platform / SOC"]  

    VM1 -.->|"JSON over UDP"| Middleware  
    VM2 -.->|"JSON over UDP"| Middleware  
    Node1 -.->|"JSON over UDP"| Middleware  
    Node2 -.->|"JSON over UDP"| Middleware  

    Middleware -.->|"JSON Data"| AnalyticsPlatform  
    Middleware -.->|"JSON Data"| MSA  

    MSA -.->|"Alerts (Syslog)"| AnalyticsPlatform  

    classDef vm fill:#e1f5fe,stroke:#01579b,stroke-width:2px;  
    classDef node fill:#e8f5e9,stroke:#1b5e20,stroke-width:2px;  
    classDef middleware fill:#fff3e0,stroke:#e65100,stroke-width:2px;  
    classDef msa fill:#fce4ec,stroke:#880e4f,stroke-width:2px;  
    classDef analytics fill:#f3e5f5,stroke:#4a148c,stroke-width:2px;  

    class VM1,VM2 vm;  
    class Node1,Node2 node;  
    class Middleware middleware;  
    class MSA msa;  
    class AnalyticsPlatform analytics;
```



## 🤷‍♂️How to Use？

