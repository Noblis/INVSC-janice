Training
========

Functions
---------

.. _janice\_train:

janice\_train
~~~~~~~~~~~~~

Train an implementation using new data.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_train(const char* data_prefix,
                                           const char* data_list);

Thread Safety
^^^^^^^^^^^^^

This function is thread unsafe.

Parameters
^^^^^^^^^^

+--------------+---------------+-----------------------------------------------+
| Name         | Type          | Description                                   |
+==============+===============+===============================================+
| data\_prefix | const char\*  | A prefix path pointing to the locations of    |
|              |               | the training data.                            |
+--------------+---------------+-----------------------------------------------+
| data\_train  | const char\*  | A list of training data and labels. The       |
|              |               | format is currently unspecified.              |
+--------------+---------------+-----------------------------------------------+

Notes
^^^^^

**This function is untested, unstable and most likely subject to breaking
changes in future releases.**
