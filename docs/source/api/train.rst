Training
========

Functions
---------

.. _janice\_fine\_tune:

janice\_fine\_tune
~~~~~~~~~~~~~~~~~~

Fine tune an implementation using new data. This function can be used to adapt
an algorithm to a new domain. It is optional and can return
_JANICE_NOT_IMPLEMENTED_. Artifacts created from fine tuning should be stored
on disk and will be loaded in a successive initialization of the API.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_fine_tune(JaniceDetections samples,
                                               int* labels,
                                               uint32_t num_samples,
                                               const char* output_prefix);

Thread Safety
^^^^^^^^^^^^^

This function is thread unsafe.

Parameters
^^^^^^^^^^

+----------------+-------------------------+----------------------------------+
| Name           | Type                    | Description                      |
+================+=========================+==================================+
| samples        | :ref:`JaniceDetections` | Training data to fine tune on.   |
+----------------+-------------------------+----------------------------------+
| labels         | int\*                   | Class labels for the training    |
|                |                         | data. This array is the same     |
|                |                         | length as _samples_.             |
+----------------+-------------------------+----------------------------------+
| num\_samples   | uint32_t                | The number of samples.           |
+----------------+-------------------------+----------------------------------+
| output\_prefix | const char\*            | A path to an existing directory  |
|                |                         | with write access for the        |
|                |                         | application. After successful    |
|                |                         | fine tuning, this directory      |
|                |                         | should be populated with all     |
|                |                         | files necessary to initialize the|
|                |                         | API. Future calls to the API can |
|                |                         | use the fine tuned algorithm by  |
|                |                         | passing _output\_prefix_ as the  |
|                |                         | _sdk\_path_ parameter in         |
|                |                         | :ref:`janice_initialize`.        |
+----------------+-------------------------+----------------------------------+
