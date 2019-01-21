import pinocchio
from pinocchio.utils import *
from pinocchio.robot_wrapper import RobotWrapper

def getTalosPathFromRos():
    '''
    Uses environment variable ROS_PACKAGE_PATH.
    Typically returns /opt/openrobots/share/talos_data
    '''
    import rospkg
    rospack = rospkg.RosPack()
    MODEL_PATH = rospack.get_path('talos_data')+'/../'
    return MODEL_PATH

def loadTalosArm(modelPath='/opt/openrobots/share',freeFloating=False):
    URDF_FILENAME = "talos_left_arm.urdf"
    URDF_SUBPATH = "/talos_data/robots/" + URDF_FILENAME
    robot = RobotWrapper.BuildFromURDF(modelPath+URDF_SUBPATH, [modelPath],
                                       pinocchio.JointModelFreeFlyer() if freeFloating else None)

    return robot

def loadTalos(modelPath='/opt/openrobots/share'):
    URDF_FILENAME = "talos_reduced_v2.urdf"
    URDF_SUBPATH = "/talos_data/urdf/" + URDF_FILENAME
    robot = RobotWrapper.BuildFromURDF(modelPath+URDF_SUBPATH, [modelPath],
                                                               pinocchio.JointModelFreeFlyer())
    return robot


def loadTalosLegs(modelPath='/opt/openrobots/share'):
    from pinocchio import JointModelFreeFlyer,JointModelRX,JointModelRY,JointModelRZ
    robot = loadTalos(modelPath=modelPath)
    legMaxId = 14

    m1 = robot.model
    m2 = pinocchio.Model()
    for j,M,name,parent,Y in zip(m1.joints,m1.jointPlacements,m1.names,m1.parents,m1.inertias):
        if j.id<legMaxId:
            jid = m2.addJoint(parent,locals()[j.shortname()](),M,name)
            assert( jid == j.id )
            m2.appendBodyToJoint(jid,Y,pinocchio.SE3.Identity())
    m2.upperPositionLimit=np.matrix([1.]*19).T
    m2.lowerPositionLimit=np.matrix([-1.]*19).T
    q2 = pinocchio.randomConfiguration(m2)

    for f in m1.frames:
        if f.parent<legMaxId: m2.addFrame(f)
            
    g2 = pinocchio.GeometryModel()
    for g in robot.visual_model.geometryObjects:
        if g.parentJoint<14:
            g2.addGeometryObject(g)

    robot.model=m2
    robot.data= m2.createData()
    robot.visual_model = g2
    robot.q0=q2
    robot.visual_data = pinocchio.GeometryData(g2)

    return robot

if __name__ == "__main__":
    print("*** TALOS ARM ***")
    print(loadTalosArm().model)
    print("*** TALOS ARM floating ***")
    print(loadTalosArm(freeFloating=True).model)
    print("*** TALOS (floating) ***")
    print(loadTalos().model)
    print("*** TALOS LEGS (floating) ***")
    print(loadTalosLegs().model)

