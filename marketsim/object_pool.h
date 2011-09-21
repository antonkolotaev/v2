#ifndef _marketsim_object_pool_h_included_
#define _marketsim_object_pool_h_included_

#include <queue>
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <marketsim/common_types.h>

namespace marketsim
{
    template <class T>
        struct object_pool : boost::noncopyable
    {
        struct Chunk 
        {
            typedef boost::array<char, sizeof(T)>   Buffer;

            T * getValue() { return reinterpret_cast<T*>(&buffer_[0]); }

            Buffer   buffer_;
            Chunk    * next_free; 
        };

        DECLARE_ARROW(object_pool);

        object_pool() : first_free_chunk_(0) {}

        ~object_pool()
        {
            for (Chunk * p = first_free_chunk_; p;)
            {
                Chunk * t = p->next_free;

                p->next_free = p;       // making it not null ==> so it will be considered as free

                p = t;
            } 

            BOOST_FOREACH(Chunk & chunk, chunks_)
            {
                if (chunk.next_free == 0)
                {
                    chunk.getValue()->~T();
                }
            }
        }

        T *  alloc()
        {
            if (first_free_chunk_)
            {
                Chunk * p = first_free_chunk_;
                first_free_chunk_ = first_free_chunk_->next_free;
                p->next_free = 0;
                return p->getValue();
            }

            chunks_.push_back(Chunk());
            chunks_.back().next_free = 0;
            return chunks_.back().getValue();
        }

        void free(T * x)
        {
            Chunk * p = reinterpret_cast<Chunk*>(x);
            p->getValue()->~T();
            p->next_free = first_free_chunk_;
            first_free_chunk_ = p;
        }

    private:
        std::deque<Chunk>  chunks_;
        Chunk   *          first_free_chunk_;
    };

    struct PlacedInPool
    {
        template <class T>
            struct apply 
            {
                typedef object_pool<T>*     type;
            };
    };
}

#endif