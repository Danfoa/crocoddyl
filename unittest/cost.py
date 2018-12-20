import unittest
import numpy as np
import crocoddyl


# class QuadraticCostTest(unittest.TestCase):
#   def setUp(self):
#     # Defined the state and control dimensions
#     self.system = crocoddyl.SpringMass(crocoddyl.EulerIntegrator(), crocoddyl.EulerDiscretizer())
#     self.n = self.system.getConfigurationDimension()
#     self.m = self.system.getTangentDimension()

#     # Create random state and control values
#     self.x = np.random.rand(self.n, 1)
#     self.u = np.random.rand(self.m, 1)

#     # Create desired state
#     x_des = np.matrix(np.zeros((self.n, 1)))

#     # Create random state and control weights
#     self.q = np.random.rand(self.n)
#     self.r = np.random.rand(self.m)

#     # Create the different cost function and them data
#     self.t_cost = crocoddyl.StateTerminalQuadraticCost(x_des)
#     self.t_data = self.t_cost.createData(self.n)
#     self.tr_cost = crocoddyl.StateResidualTerminalQuadraticCost(x_des)
#     self.tr_data = self.tr_cost.createData(self.n)
#     self.r_cost = crocoddyl.StateRunningQuadraticCost(x_des)
#     self.r_data = self.r_cost.createData(self.n, self.m)

#     # Set the state or control weights
#     self.t_cost.setWeights(self.q)
#     self.tr_cost.setWeights(self.q)
#     self.r_cost.setWeights(self.q, self.r)

#   def test_terminal_quadratic_cost_terms(self):
#     # Get the quadratic cost terms
#     pointer = id(self.t_data)
#     l = self.t_cost.l(self.system, self.t_data, self.x)
#     lx = self.t_cost.lx(self.system, self.t_data, self.x)
#     lxx = self.t_cost.lxx(self.system, self.t_data, self.x)

#     # Compute here the quadratic cost terms for comparison
#     x = self.t_cost.xr(self.system, self.t_data, self.x)
#     Q = np.diag(self.q.reshape(-1))
#     m_l = 0.5 * x.T * Q * x
#     m_lx = Q * x
#     m_lxx = Q

#     # Check the terminal term values
#     self.assertEqual(np.asscalar(l[0]), m_l, "Wrong cost value")
#     self.assertEqual(lx.all(), m_lx.all(), "Wrong Jacobian value")
#     self.assertEqual(lxx.all(), m_lxx.all(), "Wrong Hessian value")

#     # Check the correct data values
#     self.assertEqual(np.asscalar(self.t_data.l[0]), m_l, "Wrong data collection")
#     self.assertEqual(self.t_data.lx.all(), m_lx.all(), "Wrong data collection")
#     self.assertEqual(self.t_data.lxx.all(), m_lxx.all(), "Wrong data collection")

#     # Check the address of the cost data
#     self.assertEqual(pointer, id(self.t_data), "It has been changed the data address")

#   def test_terminal_residual_quadratic_cost_terms(self):
#     # Get the quadratic cost terms
#     pointer = id(self.tr_data)
#     l = self.tr_cost.l(self.system, self.tr_data, self.x)
#     lx = self.tr_cost.lx(self.system, self.tr_data, self.x)
#     lxx = self.tr_cost.lxx(self.system, self.tr_data, self.x)

#     # Compute here the quadratic cost terms for comparison
#     r = self.tr_cost.r(self.system, self.tr_data, self.x)
#     rx = self.tr_cost.rx(self.system, self.tr_data, self.x)
#     Q = np.diag(self.q.reshape(-1))
#     m_l = np.asscalar(0.5 * r.T * Q * r)
#     m_lx = rx.T * Q * r
#     m_lxx = rx.T * Q * rx

#     # Check the terminal term values
#     self.assertEqual(np.asscalar(l[0]), m_l, "Wrong cost value")
#     self.assertEqual(lx.all(), m_lx.all(), "Wrong Jacobian value")
#     self.assertEqual(lxx.all(), m_lxx.all(), "Wrong Hessian value")

#     # Check the correct data values
#     self.assertEqual(np.asscalar(self.tr_data.l[0]), m_l, "Wrong data collection")
#     self.assertEqual(self.tr_data.lx.all(), m_lx.all(), "Wrong data collection")
#     self.assertEqual(self.tr_data.lxx.all(), m_lxx.all(), "Wrong data collection")

#     # Check the address of the cost data
#     self.assertEqual(pointer, id(self.tr_data), "It has been changed the data address")

#   def test_running_quadratic_cost(self):
#     # Since running cost depends only of the state, this is equivalent to termical cost
#     # for the cost value and state-related derivatives; the control-related derivatives
#     # are zero
#     pointer = id(self.r_data)
#     l = self.r_cost.l(self.system, self.r_data, self.x, self.u)
#     lx = self.r_cost.lx(self.system, self.r_data, self.x, self.u)
#     lu = self.r_cost.lu(self.system, self.r_data, self.x, self.u)
#     lxx = self.r_cost.lxx(self.system, self.r_data, self.x, self.u)
#     luu = self.r_cost.luu(self.system, self.r_data, self.x, self.u)
#     lux = self.r_cost.lux(self.system, self.r_data, self.x, self.u)
#     m_l = self.t_cost.l(self.system, self.t_data, self.x)
#     m_lx = self.t_cost.lx(self.system, self.t_data, self.x)
#     m_lu = np.zeros((self.m, 1))
#     m_lxx = self.t_cost.lxx(self.system, self.t_data, self.x)
#     m_luu = np.zeros((self.m, self.m))
#     m_lux = np.matrix(np.zeros((self.m, self.n)))

#     # Check the running term values
#     self.assertEqual(np.asscalar(l[0]), m_l, "Wrong cost value")
#     self.assertEqual(lx.all(), m_lx.all(), "Wrong state Jacobian value")
#     self.assertEqual(lu.all(), m_lu.all(), "Wrong control Jacobian value")
#     self.assertEqual(lxx.all(), m_lxx.all(), "Wrong state Hessian value")
#     self.assertEqual(luu.all(), m_luu.all(), "Wrong control Hessian value")
#     self.assertEqual(lux.all(), m_lux.all(), "Wrong state/control cost derivatives value")

#     # Check the correct data values
#     self.assertEqual(np.asscalar(self.r_data.l[0]), m_l, "Wrong data collection")
#     self.assertEqual(self.r_data.lx.all(), m_lx.all(), "Wrong data collection")
#     self.assertEqual(self.r_data.lu.all(), m_lu.all(), "Wrong data collection")
#     self.assertEqual(self.r_data.lxx.all(), m_lxx.all(), "Wrong data collection")
#     self.assertEqual(self.r_data.luu.all(), m_luu.all(), "Wrong data collection")
#     self.assertEqual(self.r_data.lux.all(), m_lux.all(), "Wrong data collection")

#     # Check the address of the cost data
#     self.assertEqual(pointer, id(self.r_data), "It has been changed the data address")

class SE3CostTest(unittest.TestCase):
  def setUp(self):
    # Getting the robot model from the URDF file. Note that we use the URDF file
    # installed by binary (through sudo-apt install robotpkg-talos-data)
    import pinocchio as se3
    path = '/opt/openrobots/share/talos_data/'
    urdf = path + 'robots/talos_left_arm.urdf'
    robot = se3.robot_wrapper.RobotWrapper(urdf, path)

    # Create the dynamics and its integrator and discretizer
    integrator = crocoddyl.EulerIntegrator()
    discretizer = crocoddyl.EulerDiscretizer()
    dynamics = crocoddyl.ForwardDynamics(integrator, discretizer, robot.model)
    self.dynamics_data = dynamics.createData(0., 1e-3)

    # Creating the random state and control, and updating the dynamics terms
    x = np.random.rand(dynamics.nxImpl(), 1)
    u = np.random.rand(dynamics.nu(), 1)
    dynamics.updateDynamics(self.dynamics_data, x, u)
    dynamics.updateLinearAppr(self.dynamics_data, x, u)

    # Creating the SO3 cost and its equivalency with the SE3 cost
    wSO3 = np.ones((3,1))
    wSE3 = np.vstack([np.zeros((3,1)), np.ones((3,1))])
    self.so3_cost = crocoddyl.SO3Cost(dynamics, wSO3)
    self.se3_cost = crocoddyl.SE3Cost(dynamics, wSE3)
    self.so3_data = self.so3_cost.createData(dynamics)
    self.se3_data = self.se3_cost.createData(dynamics)

    # Defining randomly the SE3 and S03 reference
    t = np.random.rand(3,1)
    R = se3.utils.rpyToMatrix(np.random.rand(3,1))
    oMr = se3.SE3(R,t)
    frame_id = robot.model.getFrameId('gripper_left_joint')
    so3_ref = crocoddyl.SO3Task(oMr.rotation, frame_id)
    se3_ref = crocoddyl.SE3Task(oMr, frame_id)
    self.so3_cost.setReference(self.so3_data, so3_ref)
    self.se3_cost.setReference(self.se3_data, se3_ref)

  def test_so3_residual_against_se3(self):
    # Updating the residual for both
    self.so3_cost.updateResidual(self.so3_data, self.dynamics_data)
    self.se3_cost.updateResidual(self.se3_data, self.dynamics_data)

    for i in xrange(3):
      self.assertTrue(np.allclose(self.so3_data.r[i], self.se3_data.r[i+3]),
        "The SO3 residual isn't equal to the equivalent SE3 one.")

  def test_so3_linear_approximation_against_se3(self):
    # Updating the linear approximation of the residual for both
    self.so3_cost.updateResidualLinearAppr(self.so3_data, self.dynamics_data)
    self.se3_cost.updateResidualLinearAppr(self.se3_data, self.dynamics_data)

    for i in xrange(3):
      self.assertTrue(np.allclose(self.so3_data.rx[i,:], self.se3_data.rx[i+3,:]),
        "The linear approximation of the SO3 residual isn't equal to the equivalent SE3 one.")

if __name__ == '__main__':
  unittest.main()
