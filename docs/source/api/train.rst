Training
========

Functions
---------

.. _janice_fine_tune:

janice\_fine\_tune
~~~~~~~~~~~~~~~~~~

Fine tune an implementation using new data. This function can be used to adapt
an algorithm to a new domain. It is optional and can return
*JANICE_NOT_IMPLEMENTED*. Artifacts created from fine tuning should be stored
on disk and will be loaded in a successive initialization of the API.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_fine_tune(JaniceMediaIterators media,
                                               JaniceTracksGroup group,
                                               int** labels,
                                               const char* output_prefix);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`thread_unsafe`.

Parameters
^^^^^^^^^^

+----------------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|      Name      |            Type             |                                                                                                                                                              Description                                                                                                                                                               |
+================+=============================+========================================================================================================================================================================================================================================================================================================================================+
| media          | :ref:`JaniceMediaIterators` | A list of media objects to fine tune with.                                                                                                                                                                                                                                                                                             |
+----------------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tracks         | :ref:`JaniceTracksGroup`    | A collection of location information for objects in the fine tuning data. There must be the same number of sublists in this structure as there are elements in *media*. The tracks in the *ith* sublist of this structure give locations in the *ith* media object.                                                                    |
+----------------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| labels         | int\*\*                     | A list of lists of labels for objects in the fine tuning data. *labels[i][j]* should give the label for the *jth* track in the *ith* sublist of *tracks*.                                                                                                                                                                              |
+----------------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| output\_prefix | const char\*                | A path to an existing directory with write access for the application. After successful fine tuning, this directory should be populated with all files necessary to initialize the API. Future calls to the API can use the fine tuned algorithm by passing *output\_prefix* as the *sdk\_path* parameter in :ref:`janice_initialize`. |
+----------------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
