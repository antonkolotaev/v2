#ifndef _marketsim_object_pool_h_included_
#define _marketsim_object_pool_h_included_

#include <queue>
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <marketsim/common/macros.h>

namespace marketsim
{
    /// This class may be used to avoid excessive memory allocations/deallocations
    /// It allocates memory in std::deque of fixed-size chunks
    /// and manages singly linked list of released chunks
    template <typename T>
        struct object_pool : boost::noncopyable
    {
        struct Chunk 
        {
            typedef boost::array<char, sizeof(T)>   Buffer;

            T * getValue() { return reinterpret_cast<T*>(&buffer_[0]); }

            /// buffer that holds values of type T
            Buffer   buffer_;

            /// pointer to next free chunk if any
            Chunk    * next_free; 
        };

        DECLARE_ARROW(object_pool);

        object_pool() : first_free_chunk_(0) {}

        ~object_pool()
        {
            // marking all released chunks
            for (Chunk * p = first_free_chunk_; p;)
            {
                Chunk * t = p->next_free;

                p->next_free = p;       // making it not null ==> so it will be considered as free

                p = t;
            } 

            BOOST_FOREACH(Chunk & chunk, chunks_)
            {
                /// if a chunk is not released, call destructor for its value
                if (chunk.next_free == 0)
                {
                    //chunk.getValue()->~T();
                }
            }
        }

        /// allocates a memory for a new T; should be used with placement new
        T *  alloc()
        {
            /// if there are released chunks
            if (first_free_chunk_)
            {
                // take the first one
                Chunk * p = first_free_chunk_;
                // remove it from the list
                first_free_chunk_ = first_free_chunk_->next_free;
                // mark it as occupied
                p->next_free = 0;
                // return memory where to create a T value
                return p->getValue();
            }
            else // we have no free chunks
            {
                /// so let's create a new one
                chunks_.push_back(Chunk());
                /// mark it as occupied
                chunks_.back().next_free = 0;
                /// return memory where to create a T value
                return chunks_.back().getValue();
            }
        }

        /// frees memory occupied by *x
        void free(T * x)
        {
            /// getting a chunk from the pointer to value
            Chunk * p = reinterpret_cast<Chunk*>(x);
            /// calling destructor
            p->getValue()->~T();
            /// inserting the chunk into... 
            p->next_free = first_free_chunk_;
            /// the head of list of released chunks
            first_free_chunk_ = p;
        }

    private:
        std::deque<Chunk>  chunks_;
        Chunk   *          first_free_chunk_;
    };

    struct PlacedInPool
    {
        template <typename T>
            struct apply 
            {
                typedef object_pool<T>*     type;
            };
    };
}

#endif