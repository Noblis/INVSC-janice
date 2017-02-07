# JanICE API

The JanICE API is a *C* API that provides a common interface between computer vision
algorithms and agencies and entities that would like to use them. The API consists
of a core header file defining required *C* functions. It also defines a number of interfaces to other languages on top of the *C* API.

The documentation for the *C* API is available [here](api/software_concepts.md).

There are currently interfaces for:

There are plans to write interfaces for:

* [*C++*](interfaces/cpp.md)
* [*Python*](interfaces/python.md)
* [*C#*](interfaces/csharp.md)
* [*Java*](interfaces/java.md)

## About

<div style="float:right; display:inline-block; margin-top:-80px">
    <img src="assets/dhs_logo_small.png" alt="DHS_S&T_logo">
</div>

Computer vision is a rapidly expanding and improving field that has seen significant
progress in it's capabilities over the past decade. Government agencies can leverage
computer vision algorithms to better understand images and videos that they ingest.
This in turn can lead to improved response times, increased public safety, and numerous
other benefits. The JanICE API provides a common framework that commercial vendors and
government agencies can use to ease integration between algorithms and use cases. The
API aims to cover a number of different Computer Vision subproblems. At this time these
problems include:

* Face Recognition
* Camera Identification

Some function calls serve multiple use cases in different ways. In those cases the function documentation strives to clearly indicate the differences. If no differences are indicated it means that the function is universal in that it applies the same to each subproblem addressed by the API.

This work is being sponsored by The Department of Homeland Security; Science and Technology Directorate.

## Focus Areas

### Face Recognition
Facial recognition has emerged as a key technology for government agencies to efficiently triage and analyze large data streams. A large ecosystem of facial recognition algorithms already exists from a variety of sources including commercial vendors, government programs and academia. However, integrating this important technology into existing technology stacks is a difficult and expensive endeavor. The JanICE API aims to address this problem by functioning as a compatibility layer between users and the algorithms. Users can write their applications on "top" of the API while algorithm providers will implement their algorithms "beneath" the API. This means that users can write their applications independent of any single FR algorithm and gives them the freedom to select the algorithm or algorithms that best serve their specific use case without worrying about integration. Algorithm providers will be able to serve their algorithms across teams and agencies without having to integrate with the different tools and services of each specific team.

### Camera Identification
Camera identification provides the capability to use sensor pattern noise (SPN) to identify images (still and video) that have been captured using the same source camera device. Since the SPN generation and matching processes are similar to biometric template generation and matching, the JanICE functionality can be adapted for this purpose. Likewise the use cases are similar in that 1:1, 1:N, and clustering capabilities are needed. (The latter two implying the existence of a gallery of imagery for which SPN has been calculated.) This also allows both FR and Camera Identification modules (algorithms) to be deployed within the same application/platform.

## License
The API is provided under the MIT license(LICENSE.txt) and is
*free for academic and commercial use*.
