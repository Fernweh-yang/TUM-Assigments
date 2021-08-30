//
// Created by shubham on 08.06.21.
//

#include "constant.h"
#include "mpi.h"
#include <omp.h>
#include <iostream>
#include <array>
#include <vector>
#include <math.h>
#include <atomic>
namespace Hybrid{
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
    Hybrid::Octree* octreeStorage;
    std::atomic<size_t> octreeCounter{ 0 };
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
        Hybrid::OctreeCell thisNode;
        Body body{};
        bool isNoLeaf{ false };
        std::array<uint32_t, 8> children{};
        //Returning a size_t is faster than having a reference as param (in this case)
        size_t CaseSelector(const vec3& pos);

    public:
        Octree() = delete;
        Octree(Hybrid::OctreeCell&& o, const Body& b) : thisNode(std::move(o))
        {
            body.pos = b.pos;
            body.m = b.m;
            isNoLeaf = false;
        }
        Octree(const Hybrid::OctreeCell& o, const Body& b) : thisNode(o)
        {
            body.pos = b.pos;
            body.m = b.m;
            isNoLeaf = false;
        }
        const Hybrid::OctreeCell& getOctreeCell() const;
        void InsertBody(const Body& b);
        uint32_t* getChild(size_t nodeId)
        {
            return &children[nodeId];
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
    const Hybrid::OctreeCell& Hybrid::Octree::getOctreeCell() const {
        return thisNode;
    }
    void Hybrid::Octree::InsertBody(const Body& b)
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
                children[casenodeId] = octreeCounter++;
                new (&octreeStorage[children[casenodeId]]) Hybrid::Octree(GetCell(thisNode.getMidpointRef(), thisNode.getRadius(), casenodeId), b);

            }
            else {
                octreeStorage[children[casenodeId]].InsertBody(b);

            }
        }
        else {
            isNoLeaf = true;
            size_t casenodeId = CaseSelector(body.pos);
            children[casenodeId] = octreeCounter++;
            new (&octreeStorage[children[casenodeId]]) Hybrid::Octree(GetCell(thisNode.getMidpointRef(), thisNode.getRadius(), casenodeId), body);

            InsertBody(b);
        }
    }

    size_t Hybrid::Octree::CaseSelector(const vec3& pos) {
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

    //Somehow this function is faster than manually inlining it, but only in Tree Interact, not Body Interact
    static inline double calculateForce(double m, double m2, double dist) {
        return (G * m * m2) / ((dist * dist + SOFTENING * SOFTENING) * dist);
    }
    static inline double calculateForce(double m, double distSquared) {
        return (m) / ((distSquared + softeningNotInMetersSquared) * sqrt(distSquared));
    }
    void Hybrid::Octree::TreeInteract(Body* bod) const
    {
        //////////////////////////////////////////////////////////////////
        ///////// Computes the force on body due to all the others ///////
        /////////////////////////////////////////////////////////////////
        vec3 diff = bod->pos - body.pos;
        //Compare squares against each other
        //Saves a sqrt (instruction more expensive than a multiplication)

        double squaredDist = Hybrid::squaredMagnitude(diff);
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

        Hybrid::OctreeCell root = Hybrid::OctreeCell(0, //// x center of root
            0,       //// y center of root
            0.1374, //// z center of root
            60 * SYSTEM_SIZE
        );
        auto rootNode = new (&octreeStorage[octreeCounter++]) Hybrid::Octree(root, bodies[1]);

#pragma omp parallel for 
        for (int octant = 0; octant < 8; octant++) {
            auto subCell = GetCell(root.getMidpointRef(),root.getRadius(), octant);
            Hybrid::Octree* subRoot = nullptr;
            auto* subRootId = rootNode->getChild(octant);
            for (int bcount = 2; bcount < numBodies; bcount++) {
                // Check if the body lies in the system
                // This makes sure that the bodies that are just too far are
                // not included in the computation makes job easier
                if (subCell.contains(bodies[bcount].pos)) {
                    if (*subRootId == 0) {
                        *subRootId = octreeCounter++;
                        subRoot = new(&octreeStorage[*subRootId]) Hybrid::Octree(subCell, bodies[bcount]);
                    }
                    else
                        subRoot->InsertBody(bodies[bcount]);
                }
            }
        }
        for (int octant = 0; octant < 8; octant++) {
            auto rootNodeId = *rootNode->getChild(octant);
            auto* subRoot = &octreeStorage[rootNodeId];
            if (rootNodeId != 0) {
                auto& subRootBody = subRoot->getBody();
                auto& rootBody = rootNode->getBody();
                rootBody.pos.x = (subRootBody.pos.x * subRootBody.m + rootBody.pos.x * rootBody.m) / (rootBody.m + subRootBody.m);
                rootBody.pos.y = (subRootBody.pos.y * subRootBody.m + rootBody.pos.y * rootBody.m) / (rootBody.m + subRootBody.m);
                rootBody.pos.z = (subRootBody.pos.z * subRootBody.m + rootBody.pos.z * rootBody.m) / (rootBody.m + subRootBody.m);
                rootBody.m += subRootBody.m;
            }
        }
        rootNode->setNoLeaf(true);
        return &octreeStorage[0];
    }
    void SunInteraction(Body* bodies, const int numBodies) {

        // Interaction with sun is individual
        Body sun = bodies[0];
        double sunAccX = 0., sunAccY = 0., sunAccZ = 0.;
        for (int bcount = 1; bcount < numBodies; bcount++) {
            auto& body = bodies[bcount];
            vec3 diff = sun.pos - body.pos;
            diff = diff;

            double dist = Hybrid::squaredMagnitude(diff);
            double F = calculateForce(1, dist);

            auto tmpAcc = diff * (F * body.m);
            sunAccX -= tmpAcc.x;
            sunAccY -= tmpAcc.y;
            sunAccZ -= tmpAcc.z;
            body.acc = body.acc + diff * (F * sun.m);
        }
        bodies[0].acc = bodies[0].acc + vec3{ sunAccX, sunAccY, sunAccZ };
    }
    void TreeInteract(Body* bodies, const int numBodies, const Octree* rootNode) {

        const auto* root = &rootNode->getOctreeCell();
        // Now compute the interaction due to foces between the bodies
#pragma omp parallel for schedule(guided, 4)
        for (int bcount = workStart; bcount < workEnd; bcount++) {
            // Check if the body lies in the system
            if (root->contains(bodies[bcount].pos)) {
                rootNode->TreeInteract(&bodies[bcount]);
            }
        }
    }
    void Integrate(Body* bodies, const int numBodies) {
#pragma omp parallel for
        for (int bcount = 0; bcount < numBodies; bcount++) {
            Hybrid::Integrate(&bodies[bcount]);
        }
    }
    inline void AllGatherAccs(Body* bodies) {
        //Would be sufficient to just transmit the accelerations, a SoA would be better for that than the current AoS
        //Bytes are faster than custom mpi datatype
        MPI_Allgatherv(&bodies[workStart], workSizes[rank], MPI_BYTE, bodies, workSizes.data(), workStarts.data(), MPI_BYTE, MPI_COMM_WORLD);
    }
    void UpdateStep(Body* bodies, const int numBodies) {
        /////////////////////////////////////////////////////////////
        /// The function sets up tree and update the ////////////////
        /// positions of bodies for one time step ///////////////////
        ////////////////////////////////////////////////////////////
        //if(rank == 0)

        //if (rank == 0)
        Hybrid::CreateOctree(bodies, numBodies);
        //While Octree creation possibly parallelizable, tradeoff with broadcasting the whole tree
        //BroadCastTree();

        Hybrid::TreeInteract(bodies, numBodies, &octreeStorage[0]);

        Hybrid::AllGatherAccs(bodies);

        Hybrid::octreeCounter = 0;

        Hybrid::SunInteraction(bodies, numBodies);
        Hybrid::Integrate(bodies, numBodies);

    }

    void Solve(Body* bodies, const int numBodies, const int numSteps);
}

void Hybrid::Solve(Body *bodies, const int numBodies, const int numSteps) {
    // Implementation here
    // Hardcoded thread count if the num-threads ICV hasn't been set
    if(omp_get_num_threads() == 1)
        omp_set_num_threads(4);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int bodyCount = numBodies;
    int stepCount = numSteps;
    Body* bodyStorage;
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

    //MPI_BYTE = unsigned char
    MPI_Bcast(bodyStorage, bodyCount * sizeof(Body), MPI_BYTE, 0, MPI_COMM_WORLD);
    octreeStorage = reinterpret_cast<Hybrid::Octree*>(malloc(sizeof(Hybrid::Octree) * bodyCount * 2ull));
    octreeCounter = 0;
    for (size_t step = 1; step < static_cast<size_t>(stepCount); step++) {
        Hybrid::UpdateStep(bodyStorage, bodyCount);
    }
    if (rank != 0)
        delete[] bodyStorage;
    free(octreeStorage);
}