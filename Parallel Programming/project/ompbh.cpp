//
// Created by shubham on 08.06.21.
//
#include "math.h"
#include "constant.h"
#include <omp.h>
#include <iostream>
#include <array>
#include <vector>
#include <atomic>
#include <mutex>
#include <type_traits>
#include <cstring>

namespace OMP {
    // Variants of constants defined in constants.h
    constexpr double squaredMaxDistance = MAX_DISTANCE * MAX_DISTANCE;
    constexpr double squaredToMeters = TO_METERS * TO_METERS;
    constexpr double squaredMaxDistanceInMpowMinusOne = MAX_DISTANCE * MAX_DISTANCE / (TO_METERS * TO_METERS);
    constexpr double softeningNotInMeters = SOFTENING / TO_METERS;
    constexpr double softeningNotInMetersSquared = softeningNotInMeters * softeningNotInMeters;
    constexpr double gNotInSquaredMeters = G / squaredToMeters;
    //This define controlles whether we allocate分配 each node directly or whether we use contigous邻近的 memory
    //contigous memory Allocator(CMA)连续内存分配器
    //Shubham mentioned, that this is "dangerous territory", meaning it is maybe (probably) not allowed
    //#define USE_CONTIGUOUS_MEMORY

    //This define controlls whether we also iterate over the nodes in an array like fashion to normalize the
    //center of mass of each node. I.e. divide by the total mass, this can be easily parallelized.
    //However, this is likely also not in the sense of the exercise and only here as a reference
#ifdef USE_CONTIGUOUS_MEMORY
//#define NORMALIZE
#endif

//This define controlls whether we change the order of the calculations in a more optimized way
//Because these calculations are rather numerically数字上的 unstable and the whole system per definition chaotic混乱的
//this causes some outliers (~2.2% over 2000 timesteps) to exceed an absolute error threshold of 0.1.
//However, the MAE is still rather low with ~1e-2.
//#define MATHEMATICAL_IDENTICAL_OPTIMIZATIONS

//One way of parallelizing the octree creation is by statically静态的 dividing the work into 8 octants and
//discarding丢弃 each body which doesn't lie in this octant. This only speeds up if we don't use 
//contigous memory, otherwise否则 the synchronization/lock overhead makes this method slower
//However, this method incurs招致 significant error (~14% over 2000 timesteps) and a MAE of ~0.17  
#ifndef USE_CONTIGUOUS_MEMORY
//#define PARALLELIZE_OCTREE_CREATION
#endif

#ifdef USE_CONTIGUOUS_MEMORY
    class Octree;
    std::vector<OMP::Octree> octreeStorage;
#endif

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
    class Octree {=5
    private:
        OMP::OctreeCell thisNode;
        Body body{};
        bool isNoLeaf{ false };
#ifdef USE_CONTIGUOUS_MEMORY
        std::array<uint32_t, 8> children{};
#else
        std::array<Octree*, 8> children{};
#endif
        //Returning a size_t is faster than having a reference as param (in this case)
        size_t CaseSelector(const vec3& pos);

    public:
        Octree() = delete;
        Octree(OMP::OctreeCell&& o, const Body& b) : thisNode(std::move(o))
        {
            body.pos = b.pos;
            body.m = b.m;
            isNoLeaf = false;
        }
        Octree(const OMP::OctreeCell& o, const Body& b) : thisNode(o)
        {
            body.pos = b.pos;
            body.m = b.m;
            isNoLeaf = false;
        }
#ifndef USE_CONTIGUOUS_MEMORY
        ~Octree() {
            if (children[0] != nullptr) delete children[0];
            if (children[1] != nullptr) delete children[1];
            if (children[2] != nullptr) delete children[2];
            if (children[3] != nullptr) delete children[3];
            if (children[4] != nullptr) delete children[4];
            if (children[5] != nullptr) delete children[5];
            if (children[6] != nullptr) delete children[6];
            if (children[7] != nullptr) delete children[7];
        }
#endif
        const OMP::OctreeCell& getOctreeCell() const;
        void InsertBody(const Body& b);
#ifdef USE_CONTIGUOUS_MEMORY
        uint32_t
#else
        Octree*
#endif
            * getChild(size_t nodeId)
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
#ifdef NORMALIZE
        void NormalizeCenterOfMass();
#endif
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
    const OMP::OctreeCell& OMP::Octree::getOctreeCell() const {
        return thisNode;
    }
#ifdef NORMALIZE
    void OMP::Octree::NormalizeCenterOfMass()
    {
        if (isNoLeaf)
        {
            body.pos.x = (body.pos.x) / (body.m);
            body.pos.y = (body.pos.y) / (body.m);
            body.pos.z = (body.pos.z) / (body.m);
        }
    }
#endif
    void OMP::Octree::InsertBody(const Body& b)
    {

        if (isNoLeaf)
        {
#ifdef NORMALIZE
            body.pos.x = (b.pos.x * b.m + body.pos.x);
            body.pos.y = (b.pos.y * b.m + body.pos.y);
            body.pos.z = (b.pos.z * b.m + body.pos.z);
#else
            body.pos.x = (b.pos.x * b.m + body.pos.x * body.m) / (body.m + b.m);
            body.pos.y = (b.pos.y * b.m + body.pos.y * body.m) / (body.m + b.m);
            body.pos.z = (b.pos.z * b.m + body.pos.z * body.m) / (body.m + b.m);
#endif
            body.m += b.m;
            //A check here is redundant since we only insert nodes which are in its parent
            //and we fully divide the parents space. Hence, a body which is a node, is in one of its children

            size_t casenodeId = CaseSelector(b.pos);
            //casenode 0 is the root, hence 0 can't be a child
            if (children[casenodeId] == 0) {
#ifdef USE_CONTIGUOUS_MEMORY
                children[casenodeId] = octreeStorage.size();
                octreeStorage.emplace_back(GetCell(thisNode.getMidpointRef(), thisNode.getRadius(), casenodeId), b);
#else
                children[casenodeId] = new OMP::Octree(GetCell(thisNode.getMidpointRef(), thisNode.getRadius(), casenodeId), b);
#endif
            }
            else {
#ifdef USE_CONTIGUOUS_MEMORY
                octreeStorage[children[casenodeId]].InsertBody(b);
#else
                children[casenodeId]->InsertBody(b);
#endif
            }
        }
        else {
            isNoLeaf = true;
            size_t casenodeId = CaseSelector(body.pos);
#ifdef USE_CONTIGUOUS_MEMORY
            children[casenodeId] = octreeStorage.size();
            octreeStorage.emplace_back(GetCell(thisNode.getMidpointRef(), thisNode.getRadius(), casenodeId), body);
#else
            children[casenodeId] = new OMP::Octree(GetCell(thisNode.getMidpointRef(), thisNode.getRadius(), casenodeId), body);
#endif
#ifdef NORMALIZE
            body.pos.x = (body.pos.x) * (body.m);
            body.pos.y = (body.pos.y) * (body.m);
            body.pos.z = (body.pos.z) * (body.m);
#endif
            InsertBody(b);
        }
    }

    size_t OMP::Octree::CaseSelector(const vec3& pos) {
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
    void OMP::Octree::TreeInteract(Body* bod) const
    {
        //////////////////////////////////////////////////////////////////
        ///////// Computes the force on body due to all the others ///////
        /////////////////////////////////////////////////////////////////
        vec3 diff = bod->pos - body.pos;
        //Compare squares against each other
        //Saves a sqrt (instruction more expensive than a multiplication)
#ifndef MATHEMATICAL_IDENTICAL_OPTIMIZATIONS
        diff = diff * TO_METERS;
#endif
        double squaredDist = OMP::squaredMagnitude(diff);
        double squaredRadius = thisNode.getRadius() * thisNode.getRadius();
        if (!isNoLeaf)
        {

#ifdef MATHEMATICAL_IDENTICAL_OPTIMIZATIONS
            // Add epsilon to avoid zero division
            bod->acc = bod->acc - (diff * calculateForce(body.m, squaredDist + 1e-100));
#else
            if (squaredDist == 0)
                return;
            bod->acc = bod->acc - (diff * calculateForce(bod->m, body.m, sqrt(squaredDist)) / bod->m);
#endif
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
#ifdef MATHEMATICAL_IDENTICAL_OPTIMIZATIONS
        else if (squaredRadius < squaredMaxDistance * squaredDist)
#else
        else if (squaredRadius < (squaredMaxDistance / squaredToMeters) * squaredDist)
#endif
        {
            // Case where octree cell contains more than one element but is taken
#ifdef MATHEMATICAL_IDENTICAL_OPTIMIZATIONS
            bod->acc = bod->acc - (diff * calculateForce(body.m, squaredDist));
#else
            bod->acc = bod->acc - (diff * calculateForce(bod->m, body.m, sqrt(squaredDist)) / bod->m);
#endif

        }
        else {
            // Case where further refinement is necessary
#ifdef USE_CONTIGUOUS_MEMORY
            if (children[0] != 0) octreeStorage[children[0]].TreeInteract(bod); //22.2%
            if (children[1] != 0) octreeStorage[children[1]].TreeInteract(bod); //22.7%
            if (children[2] != 0) octreeStorage[children[2]].TreeInteract(bod); //21.7%
            if (children[3] != 0) octreeStorage[children[3]].TreeInteract(bod); //22.7%
            if (children[4] != 0) octreeStorage[children[4]].TreeInteract(bod); //11.4%
            if (children[5] != 0) octreeStorage[children[5]].TreeInteract(bod); //10.9%
            if (children[6] != 0) octreeStorage[children[6]].TreeInteract(bod); //10.5%
            if (children[7] != 0) octreeStorage[children[7]].TreeInteract(bod); //0.5%
#else
            if (children[0] != nullptr) children[0]->TreeInteract(bod); //22.2%
            if (children[1] != nullptr) children[1]->TreeInteract(bod); //22.7%
            if (children[2] != nullptr) children[2]->TreeInteract(bod); //21.7%
            if (children[3] != nullptr) children[3]->TreeInteract(bod); //22.7%
            if (children[4] != nullptr) children[4]->TreeInteract(bod); //11.4%
            if (children[5] != nullptr) children[5]->TreeInteract(bod); //10.9%
            if (children[6] != nullptr) children[6]->TreeInteract(bod); //10.5%
            if (children[7] != nullptr) children[7]->TreeInteract(bod); //0.5%
#endif
        }
    }
    void Integrate(Body* b) {
        ////////////////////////////////////////////////
        ///// Update the velocity and //////////////////
        /// position of the body ///////////////////////
        ////////////////////////////////////////////////
#ifdef MATHEMATICAL_IDENTICAL_OPTIMIZATIONS
        b->vel = b->vel + (b->acc) * (TIME_STEP * gNotInSquaredMeters);
#else
        b->vel = b->vel + (b->acc) * (TIME_STEP);
#endif
        b->acc = 0.0;
        b->pos = b->pos + (b->vel) * (TIME_STEP / TO_METERS);
    }

    Octree* CreateOctree(Body* bodies, const int numBodies) {

        OMP::OctreeCell root = OMP::OctreeCell(0, //// x center of root
            0,       //// y center of root
            0.1374, //// z center of root
            60 * SYSTEM_SIZE
        );
#ifdef USE_CONTIGUOUS_MEMORY
        octreeStorage.emplace_back(root, bodies[1]);
#else
        auto* rootNode = new OMP::Octree(root, bodies[1]);
#endif

#ifdef PARALLELIZE_OCTREE_CREATION
#pragma omp parallel for num_threads(std::min(8, omp_get_num_threads()))
        for (int octant = 0; octant < 8; octant++) {
            auto subCell = GetCell(root.getMidpointRef(),
                root.getRadius(), octant);
            auto** subRoot = rootNode->getChild(octant);
            for (int bcount = 2; bcount < numBodies; bcount++) {
                // Check if the body lies in the system
                // This makes sure that the bodies that are just too far are
                // not included in the computation makes job easier
                if (subCell.contains(bodies[bcount].pos)) {
                    if (*subRoot == nullptr)
                        *subRoot = new OMP::Octree(subCell, bodies[bcount]);
                    else
                        (*subRoot)->InsertBody(bodies[bcount]);
                }
            }
        }
        for (int octant = 0; octant < 8; octant++) {
            auto** subRoot = rootNode->getChild(octant);
            if (*subRoot != nullptr) {
                auto& subRootBody = (*subRoot)->getBody();
                auto& rootBody = rootNode->getBody();
                rootBody.pos.x = (subRootBody.pos.x * subRootBody.m + rootBody.pos.x * rootBody.m) / (rootBody.m + subRootBody.m);
                rootBody.pos.y = (subRootBody.pos.y * subRootBody.m + rootBody.pos.y * rootBody.m) / (rootBody.m + subRootBody.m);
                rootBody.pos.z = (subRootBody.pos.z * subRootBody.m + rootBody.pos.z * rootBody.m) / (rootBody.m + subRootBody.m);
                rootBody.m += subRootBody.m;
            }
        }
        rootNode->setNoLeaf(true);
#else 
        for (int bcount = 2; bcount < numBodies; bcount++) {
            // Check if the body lies in the system
            // This makes sure that the bodies that are just too far are
            // not included in the computation makes job easier
#ifdef USE_CONTIGUOUS_MEMORY
            if (octreeStorage[0].getOctreeCell().contains(bodies[bcount].pos)) {
                octreeStorage[0].InsertBody(bodies[bcount]);
            }
#else
            if (rootNode->getOctreeCell().contains(bodies[bcount].pos)) {
                rootNode->InsertBody(bodies[bcount]);
            }
#endif
        }
#endif

#ifdef USE_CONTIGUOUS_MEMORY
        return &octreeStorage[0];
#else
        return rootNode;
#endif
    }
    void SunInteraction(Body* bodies, const int numBodies) {

        // Interaction with sun is individual
        Body sun = bodies[0];
        double sunAccX = 0., sunAccY = 0., sunAccZ = 0.;
#ifdef MATHEMATICAL_IDENTICAL_OPTIMIZATIONS
#pragma omp parallel for reduction(+:sunAccX,sunAccY, sunAccZ ) firstprivate(sun)
        for (int bcount = 1; bcount < numBodies; bcount++) {
            auto& body = bodies[bcount];
            vec3 diff = sun.pos - body.pos;
            diff = diff;

            double dist = OMP::squaredMagnitude(diff);
            double F = calculateForce(1, dist);

            auto tmpAcc = diff * (F * body.m);
            sunAccX -= tmpAcc.x;
            sunAccY -= tmpAcc.y;
            sunAccZ -= tmpAcc.z;
            body.acc = body.acc + diff * (F * sun.m);
        }
#else
        for (int bcount = 1; bcount < numBodies; bcount++) {
            auto& body = bodies[bcount];
            vec3 diff = sun.pos - body.pos;
            diff = diff * TO_METERS;

            double dist = OMP::magnitude(diff);
            double F = calculateForce(body.m, sun.m, dist);

            auto tmpAcc = diff * (F / sun.m);
            sunAccX -= tmpAcc.x;
            sunAccY -= tmpAcc.y;
            sunAccZ -= tmpAcc.z;
            body.acc = body.acc + diff * (F / body.m);
        }
#endif
        bodies[0].acc = bodies[0].acc + vec3{ sunAccX, sunAccY, sunAccZ };
    }
#ifdef NORMALIZE
    void Normalize() {
#pragma omp parallel for schedule (static, (octreeStorage.size()/omp_get_num_threads()))
        for (int nodeCount = 0; nodeCount < octreeStorage.size(); nodeCount++)
            octreeStorage[nodeCount].NormalizeCenterOfMass();
    }
#endif

    void TreeInteract(Body* bodies, const int numBodies, const Octree* rootNode) {

        const auto* root = &rootNode->getOctreeCell();
        // Now compute the interaction due to foces between the bodies
#pragma omp parallel for schedule(static, (numBodies + omp_get_num_threads() - 1)/ omp_get_num_threads()) firstprivate(root)
        for (int bcount = 1; bcount < numBodies; bcount++) {
            // Check if the body lies in the system
#ifdef USE_CONTIGUOUS_MEMORY
            if (root->contains(bodies[bcount].pos)) {
                octreeStorage[0].TreeInteract(&bodies[bcount]);
            }
#else
            if (root->contains(bodies[bcount].pos)) {
                rootNode->TreeInteract(&bodies[bcount]);
            }
#endif
        }
    }

    void Integrate(Body* bodies, const int numBodies) {
#pragma omp parallel for schedule (static, (numBodies/omp_get_num_threads()))
        for (int bcount = 0; bcount < numBodies; bcount++) {
            OMP::Integrate(&bodies[bcount]);
        }
    }

    void UpdateStep(Body* bodies, const int numBodies) {
        /////////////////////////////////////////////////////////////
        /// The function sets up tree and update the ////////////////
        /// positions of bodies for one time step ///////////////////
        ////////////////////////////////////////////////////////////
        SunInteraction(bodies, numBodies);

        auto* rootNode = CreateOctree(bodies, numBodies);

#ifdef NORMALIZE
        Normalize();
#endif

        TreeInteract(bodies, numBodies, rootNode);

        // remove the tree
#ifdef USE_CONTIGUOUS_MEMORY
        octreeStorage.clear();
#else
        delete rootNode;
#endif

        Integrate(bodies, numBodies);

    }
    void Solve(Body* bodies, const int numBodies, const int numSteps);
}

void OMP::Solve(Body* bodies, const int numBodies, const int numSteps) {
    // Implementation here
    if (numSteps < 2) return;
    if (numBodies < 2) return;
#ifdef USE_CONTIGUOUS_MEMORY
    octreeStorage.reserve(numBodies * 2ull);
#endif
    for (size_t step = 1; step < static_cast<size_t>(numSteps); step++) {
        OMP::UpdateStep(bodies, numBodies);
    }
}
