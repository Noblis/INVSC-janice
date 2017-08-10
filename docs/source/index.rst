.. JanICE documentation master file, created by
   sphinx-quickstart on Mon Aug  7 18:39:41 2017.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to JanICE's documentation!
==================================

The JanICE API is a *C* API that provides a common interface between
computer vision algorithms and agencies and entities that would like to
use them. The API consists of a core header file defining required *C*
functions. It also defines a number of interfaces to other languages on
top of the *C* API.

About
-----

.. image:: assets/dhs_logo_small.png
   :width: 150px
   :align: right

Computer vision is a rapidly expanding and improving field that has seen
significant progress in it's capabilities over the past decade.
Government agencies can leverage computer vision algorithms to better
understand images and videos that they ingest. This in turn can lead to
improved response times, increased public safety, and numerous other
benefits. The JanICE API provides a common framework that commercial
vendors and government agencies can use to ease integration between
algorithms and use cases. The API aims to cover a number of different
Computer Vision subproblems. At this time, these problems include:

-  Face Recognition

Some function calls serve multiple use cases in different ways. In those
cases the function documentation strives to clearly indicate the
differences. If no differences are indicated it means that the function
is universal in that it applies the same to each subproblem addressed by
the API.

This work is being sponsored by The Department of Homeland Security;
Science and Technology Directorate.

Focus Areas
-----------

Face Recognition
~~~~~~~~~~~~~~~~

Facial recognition has emerged as a key technology for government
agencies to efficiently triage and analyze large data streams. A large
ecosystem of facial recognition algorithms already exists from a variety
of sources including commercial vendors, government programs and
academia. However, integrating this important technology into existing
technology stacks is a difficult and expensive endeavor. The JanICE API
aims to address this problem by functioning as a compatibility layer
between users and the algorithms. Users can write their applications on
"top" of the API while algorithm providers will implement their
algorithms "beneath" the API. This means that users can write their
applications independent of any single FR algorithm and gives them the
freedom to select the algorithm or algorithms that best serve their
specific use case without worrying about integration. Algorithm
providers will be able to serve their algorithms across teams and
agencies without having to integrate with the different tools and
services of each specific team.

License
-------

The API is provided under the MIT license(LICENSE.txt) and is *free for
academic and commercial use*.

.. toctree::
   :maxdepth: 1
   :caption: Contents:

   api/concepts.rst
   api/errors.rst
   api/misc.rst
   api/io.rst
   api/train.rst
   api/detection.rst
   api/enrollment.rst
   api/gallery.rst
   api/comparison.rst
   api/clustering.rst

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
