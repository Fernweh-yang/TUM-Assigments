//
// Created by shubham on 08.06.21.
//
#include <iostream>
#include <mpi.h>
#include "constant.h"
#include <array>
#include <vector>
#include <cstring>
#include <math.h>
#include <assert.h>
namespace MPI{
    class OctreeCell;
    int rank, size;
    int workSize, workStart, workEnd;
    std::vector<int> workSizes, workStarts, workEnds;
    
    constexpr double squaredMaxDistance = MAX_DISTANCE * MAX_DISTANCE;
    constexpr double squaredToMeters = TO_METERS * TO_METERS;
    constexpr double squaredMaxDistanceInMpowMinusOne = MAX_DISTANCE * MAX_DISTANCE / (TO_METERS * TO_METERS);
    constexpr double softeningNotInMeters = SOFTENING / TO_METERS;
    constexpr double softeningNotInMetersSquared = softeningNotInMeters * softeningNotInMeters;
    constexpr double gNotInSquaredMeters = G / squaredToMeters;
    ///// Implementation of helper function/class for solve here
    class Octree;
    MPI::Octree* octreeStorage;
    size_t octreeCounter;
    class OctreeCell {
    private:
        vec3 midpoint;
        double radius;

    public:
        OctreeCell() {
            midpoint = vec3(0, 0, 0);
            radius = 0.;
        }
        OctreeCell(double x, double y, double z, double rad) : midpoint(x, y, z), radius(rad) {}
        OctreeCell(OctreeCell&& o) noexcept : midpoint(std::move(o.midpoint)), radius(std::move(o.radius)) {}
        OctreeCell(const OctreeCell& o) : midpoint(o.midpoint), radius(o.radius) {}
        ~OctreeCell() {}
        OctreeCell& operator=(const OctreeCell& o) {
            if (this != &o) {
                midpoint = vec3(o.midpoint);
                radius = o.radius;
            }
            return *this;
        }
        double getRadius()const {
            return radius;
        }
        vec3 getMidpoint()const {
            return midpoint;
        }
        const vec3& getMidpointRef()const {
            return midpoint;
        }
        bool contains(const vec3& p) const {
            if (p.x > (midpoint.x + (radius / 2.0))) return false;
            if (p.x < (midpoint.x - (radius / 2.0))) return false;
            if (p.y > (midpoint.y + (radius / 2.0))) return false;
            if (p.y < (midpoint.y - (radius / 2.0))) return false;
            if (p.z > (midpoint.z + (radius / 2.0))) return false;
            if (p.z < (midpoint.z - (radius / 2.0))) return false;
            return true;
        }
        bool IsDown(const vec3& pos)const {
            return pos.z < midpoint.z;
        }
        bool IsEast(const vec3& pos)const {
            return pos.x >= midpoint.x;
        }
        bool IsSouth(const vec3& pos)const {
            return pos.y < midpoint.y;
        }
        OctreeCell GetCell(const size_t id) {
            constexpr size_t DOWN = 0b100;
            constexpr size_t SOUTH = 0b010;
            constexpr size_t EAST = 0b001;
            const double halfRadius = radius / 4.0;
            return OctreeCell(midpoint.x + (id & EAST ? halfRadius : -halfRadius),
                midpoint.y + (id & SOUTH ? -halfRadius : halfRadius),
                midpoint.z + (id & DOWN ? -halfRadius : halfRadius),
                2 * halfRadius);
        }
    };
    static inline OctreeCell GetCell(const vec3& midpoint, const double radius, const size_t id) {
        constexpr size_t DOWN = 0b100;
        constexpr size_t SOUTH = 0b010;
        constexpr size_t EAST = 0b001;
        const double halfRadius = radius / 4.0;
        return OctreeCell(midpoint.x + (id & EAST ? halfRadius : -halfRadius),
            midpoint.y + (id & SOUTH ? -halfRadius : halfRadius),
            midpoint.z + (id & DOWN ? -halfRadius : halfRadius),
            2 * halfRadius);
    }
    class Octree {
    private:
        MPI::OctreeCell thisNode;
        Body body{};
        bool isNoLeaf{ false };
        std::array<uint32_t, 8> children{};
        //Returning a size_t is faster than having a reference as param (in this case)
        size_t CaseSelector(const vec3& pos);

    public:
        Octree() = delete;
        Octree(MPI::OctreeCell&& o, const Body& b) : thisNode(std::move(o))
        {
            body.pos = b.pos;
            body.m = b.m;
            isNoLeaf = false;
        }
        Octree(const MPI::OctreeCell& o, const Body& b) : thisNode(o)
        {
            body.pos = b.pos;
            body.m = b.m;
            isNoLeaf = false;
        }
        const MPI::OctreeCell& getOctreeCell() const;
        void InsertBody(const Body& b);
        uint32_t& getChild(size_t nodeId)
        {
            return children[nodeId];
        }
        void TreeInteract(Body* bod) const;
        Body& getBody() {
            return body;
        }
        const Body& getBody() const {
            return body;
        }
        void setNoLeaf(bool val) {
            isNoLeaf = val;
        }
        void merge(const Octree& other);
    };
    inline double magnitude(double x, double y, double z)
    {
        //Assure we don't call 3 times pow
        //Compiler might optimize it but this way we are sure
        //because 3 multiplications are faster than 3 pows
        return sqrt(x * x + y * y + z * z);
    }
    inline double magnitude(const vec3& v) {
        return magnitude(v.x, v.y, v.z);
    }
    inline double squaredMagnitude(double x, double y, double z)
    {
        return x * x + y * y + z * z;
    }
    inline double squaredMagnitude(const vec3& v) {
        return squaredMagnitude(v.x, v.y, v.z);
    }
    const MPI::OctreeCell& MPI::Octree::getOctreeCell() const {
        return thisNode;
    }
    void MPI::Octree::InsertBody(const Body& b)
    {

        if (isNoLeaf)
        {
            body.pos.x = (b.pos.x * b.m + body.pos.x * body.m) / (body.m + b.m);
            body.pos.y = (b.pos.y * b.m + body.pos.y * body.m) / (body.m + b.m);
            body.pos.z = (b.pos.z * b.m + body.pos.z * body.m) / (body.m + b.m);
            body.m += b.m;
            //A check here is redundant since we only insert nodes which are in its parent
            //and we fully divide the parents space. Hence, a body which is a node, is in one of its children

            size_t casenodeId = CaseSelector(b.pos);
            //casenode 0 is the root, hence 0 can't be a child
            if (children[casenodeId] == 0) {
                children[casenodeId] = octreeCounter;
                new (&octreeStorage[octreeCounter++]) MPI::Octree(GetCell(thisNode.getMidpointRef(), thisNode.getRadius(), casenodeId), b);

            }
            else {
                octreeStorage[children[casenodeId]].InsertBody(b);

            }
        }
        else {
            isNoLeaf = true;
            size_t casenodeId = CaseSelector(body.pos);
            children[casenodeId] = octreeCounter;
            new (&octreeStorage[octreeCounter++]) MPI::Octree(GetCell(thisNode.getMidpointRef(), thisNode.getRadius(), casenodeId), body);

            InsertBody(b);
        }
    }

    size_t MPI::Octree::CaseSelector(const vec3& pos) {
        /////////////////////////////////////////////////////////////////////////////////////
        //////// Uses 3 digit binary representation to find the correct octant //////////////
        /////////////////////////////////////////////////////////////////////////////////////

        bool vertUD = thisNode.IsDown(pos);
        bool longNS = thisNode.IsSouth(pos);
        bool latEW = thisNode.IsEast(pos);
        //Changed order for branchless paths
        constexpr size_t DOWN = 0b100;
        constexpr size_t SOUTH = 0b010;
        constexpr size_t EAST = 0b001;
        return latEW * EAST + longNS * SOUTH + vertUD * DOWN;
    }

    //Somehow this function is faster than manually inlining it, but only in Tree-Body Interact, not Body-Body Interact
    static inline double calculateForce(double m, double m2, double dist) {
        return (G * m * m2) / ((dist * dist + SOFTENING * SOFTENING) * dist);
    }
    static inline double calculateForce(double m, double distSquared) {
        return (m) / ((distSquared + softeningNotInMetersSquared) * sqrt(distSquared));
    }
    void MPI::Octree::TreeInteract(Body* bod) const
    {
        //////////////////////////////////////////////////////////////////
        ///////// Computes the force on body due to all the others ///////
        /////////////////////////////////////////////////////////////////
        vec3 diff = bod->pos - body.pos;
        //Compare squares against each other
        //Saves a sqrt (instruction more expensive than a multiplication)

        double squaredDist = MPI::squaredMagnitude(diff);
        double squaredRadius = thisNode.getRadius() * thisNode.getRadius();
        if (!isNoLeaf)
        {

            // Add epsilon to avoid zero division
            bod->acc = bod->acc - (diff * calculateForce(body.m, squaredDist + 1e-100));
            //Friction
#if ENABLE_FRICTION
            double friction = 0.5 / pow(2.0, FRICTION_FACTOR * (
                ((sqrt(squaredDist) + SOFTENING)) / (TO_METERS)));
            //	cout << friction << "\n";
            if (friction > 0.0001 && ENABLE_FRICTION)
            {
                bod->acc = bod->acc + (bod->vel - body.vel) * (friction * -0.5);
            }
#endif
        }
        //We can convert a division to a multiplication here without affecting the unequality since squaredDist is always positive
        else if (squaredRadius < squaredMaxDistance * squaredDist)
        {
            // Case where octree cell contains more than one element but is taken
            bod->acc = bod->acc - (diff * calculateForce(body.m, squaredDist));
        }
        else {
            // Case where further refinement is necessary
            if (children[0] != 0) octreeStorage[children[0]].TreeInteract(bod); //22.2%
            if (children[1] != 0) octreeStorage[children[1]].TreeInteract(bod); //22.7%
            if (children[2] != 0) octreeStorage[children[2]].TreeInteract(bod); //21.7%
            if (children[3] != 0) octreeStorage[children[3]].TreeInteract(bod); //22.7%
            if (children[4] != 0) octreeStorage[children[4]].TreeInteract(bod); //11.4%
            if (children[5] != 0) octreeStorage[children[5]].TreeInteract(bod); //10.9%
            if (children[6] != 0) octreeStorage[children[6]].TreeInteract(bod); //10.5%
            if (children[7] != 0) octreeStorage[children[7]].TreeInteract(bod); //0.5%

        }
    }
    void Integrate(Body* b) {
        ////////////////////////////////////////////////
        ///// Update the velocity and //////////////////
        /// position of the body ///////////////////////
        ////////////////////////////////////////////////

        b->vel = b->vel + (b->acc) * (TIME_STEP * gNotInSquaredMeters);
        b->acc = 0.0;
        b->pos = b->pos + (b->vel) * (TIME_STEP / TO_METERS);
    }

    Octree* CreateOctree(Body* bodies, const int numBodies) {

        MPI::OctreeCell root = MPI::OctreeCell(0, //// x center of root
            0,       //// y center of root
            0.1374, //// z center of root
            60 * SYSTEM_SIZE
        );

        new (&octreeStorage[octreeCounter++]) MPI::Octree(root, bodies[1]);

        for (int bcount = 2; bcount < numBodies; bcount++) {
            // Check if the body lies in the system
            // This makes sure that the bodies that are just too far are
            // not included in the computation makes job easier

            if (root.contains(bodies[bcount].pos)) {
                octreeStorage[0].InsertBody(bodies[bcount]);
            }
        }
        return octreeStorage;
    }
    void SunInteraction(Body* bodies, const int numBodies) {

        // Interaction with sun is individual
        Body sun = bodies[0];
        double sunAccX = 0., sunAccY = 0., sunAccZ = 0.;
        for (int bcount = 1; bcount < numBodies; bcount++) {
            auto& body = bodies[bcount];
            vec3 diff = sun.pos - body.pos;
            diff = diff;

            double dist = MPI::squaredMagnitude(diff);
            double F = calculateForce(1, dist);

            auto tmpAcc = diff * (F * body.m);
            sunAccX -= tmpAcc.x;
            sunAccY -= tmpAcc.y;
            sunAccZ -= tmpAcc.z;
            body.acc = body.acc + diff * (F * sun.m);
        }
        bodies[0].acc = bodies[0].acc + vec3{ sunAccX, sunAccY, sunAccZ };
    }
    void TreeInteract(Body* bodies, const Octree* rootNode) {

        const auto* root = &rootNode->getOctreeCell();
        // Now compute the interaction due to foces between the bodies
        for (int bcount = workStart; bcount < workEnd; bcount++) {
            // Check if the body lies in the system
            if (root->contains(bodies[bcount].pos)) {
                rootNode->TreeInteract(&bodies[bcount]);
            }
        }
    }
    void Integrate(Body* bodies, const int numBodies) {
        for (int bcount = 0; bcount < numBodies; bcount++) {
            MPI::Integrate(&bodies[bcount]);
        }
    }
    inline void AllGatherAccs(Body* bodies) {
        //Would be sufficient足够的 to just transmit传达 the accelerations, a SoA would be better for that than the current AoS
        //Bytes are faster than custom mpi datatype
        //将MPI_COMM_WORLD中每个进程的bodies[workStart]串联起来存储到bodies中去
        MPI_Allgatherv(&bodies[workStart], workSizes[rank], MPI_BYTE, bodies, workSizes.data(), workStarts.data(), MPI_BYTE, MPI_COMM_WORLD);
    }
    void UpdateStep(Body* bodies, const int numBodies) {
        /////////////////////////////////////////////////////////////
        /// The function sets up tree and update the ////////////////
        /// positions of bodies for one time step ///////////////////
        ////////////////////////////////////////////////////////////
        //if(rank == 0)

        //if (rank == 0)
        MPI::CreateOctree(bodies, numBodies);

        MPI::TreeInteract(bodies, &octreeStorage[0]);

        AllGatherAccs(bodies);

        octreeCounter = 0;

        MPI::SunInteraction(bodies, numBodies);
        MPI::Integrate(bodies, numBodies);

    }

    void Solve(Body* bodies, const int numBodies, const int numSteps);
}

void MPI::Solve(Body *bodies, const int numBodies, const int numSteps) {
    // Implementation here
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int bodyCount = numBodies;
    int stepCount = numSteps;
    Body* bodyStorage;
    // 将bodyCount和stepCount中的内容发送给通信子MPI_COMM_WORLD的所有进程中去
    MPI_Bcast(&bodyCount, 1, MPI_INT32_T, 0, MPI_COMM_WORLD);
    MPI_Bcast(&stepCount, 1, MPI_INT32_T, 0, MPI_COMM_WORLD);

    workSize = (bodyCount + size - 1) / size;
    for (int i = 0; i < size; i++) {
        workStarts.push_back(std::max(workSize * i, 1)* sizeof(Body));
        workEnds.push_back(std::min(workSize * (i + 1), bodyCount) * sizeof(Body));
        workSizes.push_back(workEnds[i] - workStarts[i]);
    }
    workStart = workStarts[rank] / sizeof(Body);
    workEnd = workEnds[rank] / sizeof(Body);
    if (rank == 0)
        bodyStorage = bodies;
    else
        bodyStorage = new Body[bodyCount];

    MPI_Bcast(bodyStorage, bodyCount * sizeof(Body), MPI_BYTE, 0, MPI_COMM_WORLD);
    //reinterpret_cast<类型说明符>(表达式) 
    //static_cast<类型说明符>(表达式)
    //将表达式的值强制转换为类型性说明符的类型
    octreeStorage = reinterpret_cast<MPI::Octree*>(malloc(sizeof(MPI::Octree) * bodyCount * 2ull));
    octreeCounter = 0;
    for (size_t step = 1; step < static_cast<size_t>(stepCount); step++) {
        MPI::UpdateStep(bodyStorage, bodyCount);
    }
    if (rank != 0)
        delete[] bodyStorage;
    free(octreeStorage);
}
