#ifndef _marketsim_history_in_file_h_included_
#define _marketsim_history_in_file_h_included_

#include <fstream>

namespace marketsim{
namespace history {

	struct InFile 
	{
		InFile(const char * filename) : out_(filename) {}

        //to be replaced by std::ostream& operator << (std::ostream&, TimeStamped<T> const &);
		template <typename FieldType>
			void write(Time t, FieldType const & x)
		{
			out_ << t << "\t" << x << "\n";
		}			

	private:
		std::ofstream	out_;
	};

}}

#endif