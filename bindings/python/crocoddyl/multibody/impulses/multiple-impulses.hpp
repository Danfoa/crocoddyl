///////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (C) 2018-2019, LAAS-CNRS
// Copyright note valid unless otherwise stated in individual files.
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#ifndef BINDINGS_PYTHON_CROCODDYL_MULTIBODY_IMPULSES_MULTIPLE_IMPULSES_HPP_
#define BINDINGS_PYTHON_CROCODDYL_MULTIBODY_IMPULSES_MULTIPLE_IMPULSES_HPP_

#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <string>
#include "crocoddyl/multibody/impulses/multiple-impulses.hpp"

namespace crocoddyl {
namespace python {

namespace bp = boost::python;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(ImpulseModelMultiple_calcDiff_wraps, ImpulseModelMultiple::calcDiff_wrap, 2, 3)

void exposeImpulseMultiple() {
  // Register custom converters between std::map and Python dict
  typedef boost::shared_ptr<ImpulseDataAbstract> ImpulseDataPtr;
  bp::to_python_converter<std::map<std::string, ImpulseItem, std::less<std::string>,
                                   std::allocator<std::pair<const std::string, ImpulseItem> > >,
                          map_to_dict<std::string, ImpulseItem> >();
  bp::to_python_converter<std::map<std::string, ImpulseDataPtr, std::less<std::string>,
                                   std::allocator<std::pair<const std::string, ImpulseDataPtr> > >,
                          map_to_dict<std::string, ImpulseDataPtr, false> >();
  dict_to_map<std::string, ImpulseItem>().from_python();
  dict_to_map<std::string, ImpulseDataPtr>().from_python();

  bp::class_<ImpulseItem, boost::noncopyable>(
      "ImpulseItem", "Describe a impulse item.\n\n",
      bp::init<std::string, ImpulseModelAbstract*>(
          bp::args(" self", " name", " impulse"),
          "Initialize the impulse item.\n\n"
          ":param name: impulse name\n"
          ":param impulse: impulse model")[bp::with_custodian_and_ward<1, 3>()])
      .def_readwrite("name", &ImpulseItem::name, "impulse name")
      .add_property("impulse", bp::make_getter(&ImpulseItem::impulse, bp::return_internal_reference<>()),
                    "impulse model");

  bp::class_<ImpulseModelMultiple, boost::noncopyable>(
      "ImpulseModelMultiple",
      bp::init<StateMultibody&>(bp::args(" self", " state"),
                                "Initialize the multiple impulse model.\n\n"
                                ":param state: state of the multibody system")[bp::with_custodian_and_ward<1, 2>()])
      .def("addImpulse", &ImpulseModelMultiple::addImpulse, bp::with_custodian_and_ward<1, 3>(),
           bp::args(" self", " name", " impulse"),
           "Add a impulse item.\n\n"
           ":param name: impulse name\n"
           ":param impulse: impulse model")
      .def("removeImpulse", &ImpulseModelMultiple::removeImpulse, bp::args(" self", " name"),
           "Remove a impulse item.\n\n"
           ":param name: impulse name")
      .def("calc", &ImpulseModelMultiple::calc_wrap, bp::args(" self", " data", " x"),
           "Compute the total impulse Jacobian and drift.\n\n"
           "The rigid impulse model throught acceleration-base holonomic constraint\n"
           "of the impulse frame placement.\n"
           ":param data: impulse data\n"
           ":param x: state vector")
      .def("calcDiff", &ImpulseModelMultiple::calcDiff_wrap,
           ImpulseModelMultiple_calcDiff_wraps(
               bp::args(" self", " data", " x", " recalc=True"),
               "Compute the derivatives of the total impulse holonomic constraint.\n\n"
               "The rigid impulse model throught acceleration-base holonomic constraint\n"
               "of the impulse frame placement.\n"
               ":param data: cost data\n"
               ":param x: state vector\n"
               ":param recalc: If true, it updates the impulse Jacobian and drift."))
      .def("updateLagrangian", &ImpulseModelMultiple::updateLagrangian, bp::args(" self", " data", " lambda"),
           "Convert the Lagrangian into a stack of spatial forces.\n\n"
           ":param data: cost data\n"
           ":param lambda: Lagrangian vector")
      .def("createData", &ImpulseModelMultiple::createData, bp::with_custodian_and_ward_postcall<0, 2>(),
           bp::args(" self", " data"),
           "Create the total impulse data.\n\n"
           ":param data: Pinocchio data\n"
           ":return total impulse data.")
      .add_property(
          "impulses",
          bp::make_function(&ImpulseModelMultiple::get_impulses, bp::return_value_policy<bp::return_by_value>()),
          "stack of impulses")
      .add_property("state", bp::make_function(&ImpulseModelMultiple::get_state, bp::return_internal_reference<>()),
                    "state of the multibody system")
      .add_property("ni",
                    bp::make_function(&ImpulseModelMultiple::get_ni, bp::return_value_policy<bp::return_by_value>()),
                    "dimension of the total impulse vector");

  bp::class_<ImpulseDataMultiple, boost::shared_ptr<ImpulseDataMultiple>, bp::bases<ImpulseDataAbstract> >(
      "ImpulseDataMultiple", "Data class for multiple impulses.\n\n",
      bp::init<ImpulseModelMultiple*, pinocchio::Data*>(
          bp::args(" self", " model", " data"),
          "Create multiimpulse data.\n\n"
          ":param model: multiimpulse model\n"
          ":param data: Pinocchio data")[bp::with_custodian_and_ward<1, 2, bp::with_custodian_and_ward<1, 3> >()])
      .add_property("impulses",
                    bp::make_getter(&ImpulseDataMultiple::impulses, bp::return_value_policy<bp::return_by_value>()),
                    "stack of impulses data")
      .def_readwrite("fext", &ImpulseDataMultiple::fext, "external spatial forces");
}

}  // namespace python
}  // namespace crocoddyl

#endif  // BINDINGS_PYTHON_CROCODDYL_MULTIBODY_IMPULSES_MULTIPLE_IMPULSES_HPP_