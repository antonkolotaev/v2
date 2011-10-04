#ifndef _marketsim_python_fast_PnL_quantity_history_in_deque_h_included_
#define _marketsim_python_fast_PnL_quantity_history_in_deque_h_included_

#include <marketsim/agent/on_order_partially_filled.h>
#include <marketsim/history.h>
#include <marketsim/agent/PnL.h>
#include <marketsim/agent/quantity.h>

namespace marketsim
{
   struct Get_PnL_Quantity
   {
       // TODO: Use PnL_Quantity
       typedef PriceVolume	ValueType;

       template <typename T> 
       static ValueType getValue(T x) 
       {
           return PriceVolume(x->getPnL(), x->getQuantity());
       }
#ifdef MARKETSIM_BOOST_PYTHON
       static std::string py_name() 
       {
           return "PnL_Quantity";
       }
#endif
   };

   struct py_PnL_Quantity
   {
       typedef PriceVolume  ValueType;

       static ValueType getValue(boost::python::object x)
       {
           return 
               PriceVolume(
                    boost::python::extract<Price>(x.attr("PnL")),
                    boost::python::extract<Volume>(x.attr("assetsAvailable")));
       }
       static std::string py_name() 
       {
           return "PnL_Quantity";
       }
   };

   template <typename Base>
    struct PnL_Quantity_History_InDeque : 
       OnPartiallyFilled   < history::Collector<Get_PnL_Quantity, history::InDeque<PriceVolume> >,
       PnL_Holder          <
       Quantity_Holder     < Base 
       > > >
   {
       typedef history::Collector<Get_PnL_Quantity, history::InDeque<PriceVolume> >  TAG;

       typedef 
           OnPartiallyFilled   < TAG,
           PnL_Holder          <
           Quantity_Holder     < Base 
           > > >
           RealBase;

       DECLARE_BASE(PnL_Quantity_History_InDeque);

       template <typename T> 
        PnL_Quantity_History_InDeque(T const & x)
            :  RealBase(boost::make_tuple(x, dummy))
        {}

        history::TimeSerie<PriceVolume> const & getHistory() const 
        {
            TAG * T = 0;
            const_cast<PnL_Quantity_History_InDeque*>(this)->getHandler(T).flush();
            return getHandler(T).getHistory();
        }

        void recordHistory(bool b)
        {
            getHandler((TAG*)0).recordHistory(b);
        }

#ifdef MARKETSIM_BOOST_PYTHON
        template <typename T>
            static void py_visit(T & class_def)
            {
                RealBase::py_visit(class_def);
                class_def
                    .def("PnL_Quantity_History", &PnL_Quantity_History_InDeque::getHistory, return_internal_reference<>())
                    .def("record_PnL_Quantity_History", &PnL_Quantity_History_InDeque::recordHistory)
                    ;
            }
#endif
   };

}

#endif