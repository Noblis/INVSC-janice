Miscellaneous
=============

Functions
---------

.. _janice_initialize:

janice\_initialize
~~~~~~~~~~~~~~~~~~

Initialize global or shared state for the implementation. This function
should be called once at the start of the application, before making any
other calls to the API.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_initialize(const char* sdk_path,
                                                const char* temp_path,
                                                const char* algorithm,
                                                const int num_threads,
                                                const int* gpus,
                                                const int num_gpus);

Thread Safety
^^^^^^^^^^^^^

This function is thread unsafe.

Parameters
^^^^^^^^^^

+--------------+---------------+-----------------------------------------------+
| Name         | Type          | Description                                   |
+==============+===============+===============================================+
| sdk\_path    | const char\*  | Path to a *read-only* directory containing    |
|              |               | the JanICE compliant SDK as specified by the  |
|              |               | implementor.                                  |
+--------------+---------------+-----------------------------------------------+
| temp\_path   | const char\*  | Path to an existing empty *read-write*        |
|              |               | directory for use as temporary file storage   |
|              |               | by the implementation. This path must be      |
|              |               | guaranteed until :ref:`janice\_finalize`.     |
+--------------+---------------+-----------------------------------------------+
| algorithm    | const char\*  | An empty string indicating the default        |
|              |               | algorithm, or an implementation defined       |
|              |               | containing an alternative configuration.      |
+--------------+---------------+-----------------------------------------------+
| num\_threads | const int     | The number of threads the implementation is   |
|              |               | allowed to use. A value of '-1' indicates     |
|              |               | that the implementation should use all        |
|              |               | available hardware.                           |
+--------------+---------------+-----------------------------------------------+
| gpus         | const int\*   | A list of indices of GPUs available to the    |
|              |               | implementation. The length of the list is     |
|              |               | given by *num\_gpus*. If the implementor does |
|              |               | not require a GPU in their solution they can  |
|              |               | ignore this parameter.                        |
+--------------+---------------+-----------------------------------------------+
| num\_gpus    | const int     | The length of the *gpus* array. If no GPUs    |
|              |               | are available this should be set to 0.        |
+--------------+---------------+-----------------------------------------------+

.. _janice\_api\_version:

janice\_api\_version
~~~~~~~~~~~~~~~~~~~~

Query the implementation for the version of the JanICE API it was designed to 
implement. See :ref:`versioning` for more information on the versioning
convention for this API.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_api_version(uint32_t* major,
                                                 uint32_t* minor,
                                                 uint32_t* patch);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+---------+---------------+------------------------------------------+
| Name    | Type          | Description                              |
+=========+===============+==========================================+
| major   | uint32\_t\*   | The supported major version of the API   |
+---------+---------------+------------------------------------------+
| minor   | uint32\_t\*   | The supported minor version of the API   |
+---------+---------------+------------------------------------------+
| patch   | uint32\_t\*   | The supported patch version of the API   |
+---------+---------------+------------------------------------------+

janice\_sdk\_version
~~~~~~~~~~~~~~~~~~~~

Query the implementation for its SDK version.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_sdk_version(uint32_t* major,
                                                 uint32_t* minor,
                                                 uint32_t* patch);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+---------+---------------+--------------------------------+
| Name    | Type          | Description                    |
+=========+===============+================================+
| major   | uint32\_t\*   | The major version of the SDK   |
+---------+---------------+--------------------------------+
| minor   | uint32\_t\*   | The minor version of the SDK   |
+---------+---------------+--------------------------------+
| patch   | uint32\_t\*   | The patch version of the SDK   |
+---------+---------------+--------------------------------+

.. _janice\_finalize:

janice\_finalize
~~~~~~~~~~~~~~~~

Destroy any resources created by :ref:`janice\_initialize` and finalize the
application. This should be called once after all other API calls.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_finalize();

Thread Safety
^^^^^^^^^^^^^

This function is thread unsafe.
