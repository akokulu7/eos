/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Danny van Dyk
 *
 * This file is part of the EOS project. EOS is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * EOS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef EOS_GUARD_SRC_UTILS_SCAN_FILE_HH
#define EOS_GUARD_SRC_UTILS_SCAN_FILE_HH 1

#include <eos/utils/exception.hh>
#include <eos/utils/private_implementation_pattern.hh>
#include <eos/utils/wrapped_forward_iterator.hh>

#include <vector>

namespace eos
{
    // forward declaration
    struct HDF5File;
    typedef int hid_t;

    /*!
     * ScanFileError is parent to all exceptions thrown by ScanFile when creating, opening or accessing scan file data.
     */
    class ScanFileError :
        public Exception
    {
        public:
            /*!
             * Constructor.
             *
             * @param message The error message.
             */
            ScanFileError(const std::string & message);
    };

    /*!
     * ScanFileHDF5Error is thrown when an error occurs in interfacing libhdf5 during any
     * of ScanFile's operations.
     */
    class ScanFileHDF5Error :
        public ScanFileError
    {
        public:
            /*!
             * Constructor.
             *
             * @param function Name of the HDF5 function that failed.
             * @param code     Error code returned by the failing function.
             */
            ScanFileHDF5Error(const std::string & function, int code);
    };

    /*!
     * ScanFile represents an HDF5 formatted file that contains EOS scan data.
     *
     * An HDF5 DDL representation of a scan file follows:
     *
     * @code
     *   HDF5 "<FILE>" {
     *   GROUP "/" {
     *     GROUP "metadata" {
     *       DATASET "creator" {
     *         DATATYPE H5T_STRING {
     *           STRSIZE <CREATORLEN>
     *           STRPAD H5T_STR_NULLTERM
     *           CSET H5T_CSET_ASCII
     *           CTYPE H5T_C_S1
     *         }
     *         DATASPACE SCALAR
     *         DATA {
     *             "<CREATOR>"
     *         }
     *       }
     *       DATASET "eos_version" {
     *         DATATYPE H5T_STRING {
     *           STRSIZE <EOSVERSIONLEN>
     *           STRPAD H5T_STR_NULLTERM
     *           CSET H5T_CSET_ASCII
     *           CTYPE H5T_C_S1
     *         }
     *         DATASPACE SCALAR
     *         DATA {
     *             "<EOSVERSION>"
     *         }
     *       }
     *     }
     *     GROUP "data" {
     *       DATASET "<RESULT1>" {
     *         DATATYPE H5T_IEEE_F64LE
     *         DATASPACE SIMPLE { ( <RECORDS>, <FIELDS> ) / ( UNLIMITED, <FIELDS> ) }
     *         DATA {
     *           ...
     *         }
     *       }
     *       DATASET "<RESULT2>" {
     *         ...
     *       }
     *     }
     *   }
     *   }
     * @endcode
     *
     * Here, <FILE> is the filename, <RECORDS> is the number of scan records of <FIELDS> record fields
     * each. <CREATOR> is an identifier of the creating program, and <EOSVERSION> is a string
     * representation of the EOS version/revision that was used to create the file.
     */
    class ScanFile :
        public PrivateImplementationPattern<ScanFile>
    {
        private:
            /// Constructor.
            ScanFile(Implementation<ScanFile> * imp);

        public:
            class DataSet;
            class FieldInfo;
            class Record;
            class WriteBuffer;

            ///@name Basic Functions
            ///@{
            /// Destructor.
            ~ScanFile();

            /*!
             * Named constructor to create a new file.
             *
             * @param filename   File which shall be created.
             * @param creator    Name of the creating program.
             */
            static ScanFile Create(const std::string & filename, const std::string & creator);

            /*!
             * Named constructor to open an existing file.
             *
             * @param file_name File which shall be opened.
             */
            static ScanFile Open(const std::string & file_name);
            ///@}

            ///@name Metadata Access
            ///@{
            /// Retrieve the file creator's name.
            const std::string & creator() const;

            /// Retrieve the file creator's version information.
            const std::string & eos_version() const;

            /// Retrieve the file's name.
            const std::string & file_name() const;
            ///@}

            ///@name Data Access
            ///@{
            /*!
             * Retrieve a data set by its name.
             *
             * @param index Index of the data set that shall be retrieved.
             */
            DataSet operator[] (const std::string & name);

            /*!
             * Create a new data set by name.
             *
             * @param name   Name of the new data set.
             * @param fields Numer of fields in each record of the new data set.
             */
            DataSet add(const std::string & name, unsigned fields);
            ///@}

            ///@name Iteration
            ///@{
            struct IteratorTag;
            typedef WrappedForwardIterator<IteratorTag, DataSet> Iterator;

            Iterator begin();
            Iterator end();
            ///@}
    };

    /*!
     * ScanFile::DataSet represents one of the data sets in the group "/data" within a ScanFile.
     */
    class ScanFile::DataSet :
        public PrivateImplementationPattern<ScanFile::DataSet>
    {
        private:
            DataSet(const std::shared_ptr<HDF5File> & imp, const std::string & name);
            DataSet(const std::shared_ptr<HDF5File> & imp, const std::string & name, unsigned fields);

        public:
            friend class ScanFile;
            friend class Implementation<ScanFile>;
            friend ScanFile::DataSet & operator<< (ScanFile::DataSet &, const std::vector<double> &);
            friend ScanFile::DataSet & operator<< (ScanFile::DataSet &, const ScanFile::WriteBuffer &);

            ///@name Basic Functions
            ///@{
            /// Destructor.
            ~DataSet();
            ///@}

            ///@name Metadata Access
            ///@{
            /// Name of the data set
            std::string name() const;

            /// Number of columns in data set
            unsigned fields() const;

            /// Number of rows in data set
            unsigned records() const;

            /// Iterate over field info in data set.
            ///@{
            struct FieldIteratorTag;
            typedef WrappedForwardIterator<FieldIteratorTag, ScanFile::FieldInfo> FieldIterator;

            FieldIterator begin_fields();
            FieldIterator end_fields();
            ///@}

            /*!
             * Retrieve a field index by name.
             *
             * @param name The name of the field whose index shall be retrieved.
             */
            unsigned find_field_index(const std::string & name) const;
            ///@}

            ///@name Data Access
            ///@{
            /*!
             * Retrieve a records
             *
             * @param index Index of the record that shall be retrieved.
             */
            Record operator[] (const unsigned & index);
            ///@}
    };

    /*!
     * Holds information on one of the fields in a Scanfile::DataSet.
     */
    class ScanFile::FieldInfo :
        public PrivateImplementationPattern<ScanFile::FieldInfo>
    {
        private:
            ///@name Basic Functions
            ///@{
            /*!
             * Constructor.
             *
             * @param file   The HDF5 file that we work on.
             * @param set_id The HDF5 data set id whose field we describe.
             * @param index  The index of the field we describe.
             */
            FieldInfo(const std::shared_ptr<HDF5File> & file, hid_t set_id, unsigned index);
            ///@}

        public:
            ///@name Basic Functions
            ///@{
            /*!
             * Named constructor to create a new FieldInfo.
             *
             * @param file   The HDF5 file that we work on.
             * @param set_id The HDF5 data set id whose field we describe.
             * @param index  The index of the field we describe.
             */
            static FieldInfo Create(const std::shared_ptr<HDF5File> & file, hid_t set_id, unsigned index);

            /*!
             * Named constructor to open an existing FieldInfo from disk.
             *
             * @param file   The HDF5 file that we work on.
             * @param set_id The HDF5 data set id whose field we describe.
             * @param index  The index of the field we describe.
             */
            static FieldInfo Open(const std::shared_ptr<HDF5File> & file, hid_t set_id, unsigned index);

            /// Destructor.
            ~FieldInfo();
            ///@}

            ///@name Metadata
            ///@{
            /// Obtain the name of the field.
            std::string name() const;

            /// Set the name of the field.
            void name(const std::string & name);
            ///@}

            ///@name Numeric Data
            ///@{
            /*!
             * Obtain the contents of a named numeric attribute.
             *
             * Known attribute names are:
             *
             *   min, max, nuisance, discrete
             *
             * @param attribute_name The name of the attribute.
             * @param default_value  The value that shall be returned in case the attribute does not exist.
             */
            double get(const std::string & attribute_name, const double & default_value = 0);

            /*!
             * Write the contents of a named attribute.
             *
             * See 'get' for a list of known attribute names.
             *
             * @param attribute_name The name of the attribute.
             * @param value          The new value for the attribute.
             */
            void set(const std::string & attribute_name, const double & value);
            ///@}

            ///@name Iteration
            ///@{
            struct AttributeIteratorTag;
            typedef WrappedForwardIterator<AttributeIteratorTag, std::pair<const std::string, double>> AttributeIterator;

            AttributeIterator begin_attributes();
            AttributeIterator end_attributes();
            ///@}
    };

    /*!
     * Append a record to a ScanFile::DataSet.
     *
     * @param lhs The ScanFile::DataSet to which shall be written.
     * @param rhs The record which shall be appended.
     */
    ScanFile::DataSet & operator<< (ScanFile::DataSet & lhs, const std::vector<double> & rhs);

    /*!
     * Append an entire write buffer to a ScanFile::DataSet.
     *
     * @param lhs The ScanFile::DataSet to which shall be written.
     * @param rhs The write buffer of records which shall be appended.
     */
    ScanFile::DataSet & operator<< (ScanFile::DataSet & lhs, const ScanFile::WriteBuffer & rhs);

    /*!
     * ScanFile::Record represents one of the scan records within a ScanFile::DataSet.
     */
    class ScanFile::Record :
        public PrivateImplementationPattern<ScanFile::Record>
    {
        private:
            Record(const std::shared_ptr<Implementation<ScanFile::DataSet>> & imp, const unsigned & index);

        public:
            friend class ScanFile;
            friend ScanFile::WriteBuffer & operator<< (ScanFile::WriteBuffer &, const ScanFile::Record &);

            ///@name Basic Functions
            ///@{
            /// Destructor.
            ~Record();
            ///@}

            ///@name Data Access
            ///@{
            /// Retrieve the underlying buffer.
            const std::vector<double> & data() const;

            /// Advance the scan record by one within the parent ScanFile::DataSet.
            Record & operator++ ();

            /*!
             * Retrieve a field element.
             *
             * @param index Index of the field element that shall be retrieved.
             */
            double operator[] (const unsigned & index) const;
            ///@}
    };

    /*!
     * ScanFile::WriteBuffer temporarily keeps data which shall be written to one of the data sets within a ScanFile.
     */
    class ScanFile::WriteBuffer :
        public PrivateImplementationPattern<ScanFile::WriteBuffer>
    {
        public:
            friend ScanFile::DataSet & operator<< (ScanFile::DataSet & lhs, const ScanFile::WriteBuffer & rhs);
            friend ScanFile::WriteBuffer & operator<< (ScanFile::WriteBuffer &, const std::vector<double> &);
            friend ScanFile::WriteBuffer & operator<< (ScanFile::WriteBuffer &, const ScanFile::Record &);

            ///@name Basic Functions
            ///@{
            /*!
             * Constructor.
             *
             * @param fields   Number of fields per record.
             * @param capacity Number of records that can be held before reallocation is needed.
             */
            WriteBuffer(const unsigned & fields, const unsigned & capacity = 1024);

            /// Destructor.
            ~WriteBuffer();
            ///@}

            ///@name Access
            ///@{
            /// Retrieve the maximal number of records that can be stored in the buffer.
            unsigned capacity() const;

            /// Remove all records from the buffer.
            void clear();

            /// Retrieve the number of records currently stored in the buffer.
            unsigned size() const;

            /*!
             * Increase the buffer's capacity.
             *
             * @param capacity The number of records that shall be storable from now on.
             */
            void reserve(const unsigned & capacity);
            ///@}
    };

    /*!
     * Append a record to a ScanFile::WriteBuffer.
     *
     * @param lhs The ScanFile::WriteBuffer to which shall be written.
     * @param rhs The record which shall be appended.
     */
    ScanFile::WriteBuffer & operator<< (ScanFile::WriteBuffer & lhs, const std::vector<double> & rhs);

    /*!
     * Append a ScanFile::Record to a ScanFile::WriteBuffer.
     *
     * @param lhs The ScanFile::WriteBuffer to which shall be written.
     * @param rhs The Record object which shall be appended.
     */
    ScanFile::WriteBuffer & operator<< (ScanFile::WriteBuffer & lhs, const ScanFile::Record & rhs);
}

#endif