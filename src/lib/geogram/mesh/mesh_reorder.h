/*
 *  Copyright (c) 2000-2022 Inria
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
 */

#ifndef GEOGRAM_MESH_MESH_REORDER
#define GEOGRAM_MESH_MESH_REORDER

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/memory.h>
#include <geogram/basic/geometry.h>

#ifndef GEOGRAM_PSM
#include <geogram/mesh/mesh.h>
#endif

/**
 * \file geogram/mesh/mesh_reorder.h
 * \brief Reorders the elements in a mesh to improve data locality
 */

namespace GEO {


#ifndef GEOGRAM_PSM

    /**
     * \brief Strategy for spatial sorting.
     */
    enum MeshOrder {
        /**
         * Hilbert ordering improves data locality and
         * has a continuous mapping between indices and space.
         */
        MESH_ORDER_HILBERT,
        /**
         * Morton ordering improves data locality and is
         * a bit simpler than Hilbert ordering.
         */
        MESH_ORDER_MORTON
    };

    /**
     * \brief Reorders all the elements of a mesh.
     * \details It is used for both improving data locality
     *  and for implementing mesh partitioning.
     * \param[in] M the mesh to reorder
     * \param[in] order the reordering scheme, one of MESH_ORDER_HILBERT,
     *  MESH_ORDER_MORTION
     */
    void GEOGRAM_API mesh_reorder(
        Mesh& M, MeshOrder order = MESH_ORDER_HILBERT
    );

    /**
     * \brief Computes the spatial order of mesh elements in 3d
     * \pre M.vertices.dimension() >= 3
     * \details A "dry run" version of mesh_reorder() that leaves the mesh
     *  unchanged but that returns the reordering. Works only for meshes of
     *  dimension >= 3. If dimension is larger than 3, then the first three
     *  coordinates are used as 3d coordinates to compute the order.
     * \param[in] M the mesh
     * \param[in] elements one of MESH_VERTICES, MESH_FACETS, MESH_CELLS
     * \param[out] permutation the computed spatial order of the mesh elements.
     *   Can be subsequently used by MeshElements::permute_elements().
     * \param[in] order the reordering scheme, one of MESH_ORDER_HILBERT,
     *  MESH_ORDER_MORTION
     */
    void GEOGRAM_API compute_mesh_elements_spatial_order(
	const Mesh& M, MeshElementsFlags elements, vector<index_t>& permutation,
	MeshOrder order = MESH_ORDER_HILBERT
    );

#endif

    /**
     * \brief Computes the Hilbert order for a set of 3D points.
     * \details
     *  This variant sorts a subsequence of the indices vector.
     *  The implementation is inspired by:
     *  - Christophe Delage and Olivier Devillers. Spatial Sorting.
     *   In CGAL User and Reference Manual. CGAL Editorial Board,
     *   3.9 edition, 2011
     * \param[in] total_nb_vertices total number of vertices
     *   in the \p vertices array, used to test indices in debug mode
     * \param[in] vertices pointer to the coordinates of the vertices
     * \param[in,out] sorted_indices a vector of vertex indices, sorted
     *  spatially on exit
     * \param[in] first index of the first element in \p sorted_indices
     *  to be sorted
     * \param[in] last one position past the index of the last element
     *  in \p sorted_indices to be sorted
     * \param[in] dimension number of vertices coordinates
     * \param[in] stride number of doubles between two consecutive vertices
     */
    void GEOGRAM_API compute_Hilbert_order(
        index_t total_nb_vertices, const double* vertices,
        vector<index_t>& sorted_indices,
        index_t first,
        index_t last,
        index_t dimension, index_t stride = 3
    );

    /**
     * \brief Computes the BRIO order for a set of 3D points.
     * \details It is used to accelerate incremental insertion
     *  in Delaunay triangulation. See the following reference:
     *  -Incremental constructions con brio. Nina Amenta, Sunghee Choi,
     *   Gunter Rote, Symposium on Computational Geometry conf. proc.,
     *   2003
     * \param[in] nb_vertices number of vertices to sort
     * \param[in] vertices pointer to the coordinates of the vertices
     * \param[out] sorted_indices a vector of element indices to
     *  be sorted spatially
     * \param[in] dimension number of vertices coordinates
     * \param[in] stride number of doubles between two consecutive vertices
     * \param[in] threshold minimum size of interval to be sorted
     * \param[in] ratio splitting ratio between current interval and
     *  the rest to be sorted
     * \param[out] levels if non-nullptr, indices that correspond to level l are
     *   in the range levels[l] (included) ... levels[l+1] (excluded)
     */
    void GEOGRAM_API compute_BRIO_order(
        index_t nb_vertices, const double* vertices,
        vector<index_t>& sorted_indices,
        index_t dimension,
        index_t stride = 3,
        index_t threshold = 64,
        double ratio = 0.125,
        vector<index_t>* levels = nullptr
    );

    /**
     * \brief Spatially sort a set of vertices in periodic space.
     * \param[in] nb_vertices total number of vertices, including the
     *  virtual periodic copies. This is 27 times the number of stored vertices.
     * \param[in] vertices pointer to the coordinates of the vertices
     * \param[in,out] sorted_indices a vector of vertex indices, sorted
     * \param[in] dimension number of vertices coordinates. Only 3 is supported.
     * \param[in] stride number of doubles between two consecutive vertices
     *  spatially on exit
     * \param[in] first index of the first element in \p sorted_indices
     *  to be sorted
     * \param[in] last one position past the index of the last element
     *  in \p sorted_indices to be sorted
     * \param[in] period the translation vector to be applied in periodic mode
     */
    void GEOGRAM_API Hilbert_sort_periodic(
        index_t nb_vertices, const double* vertices,
        vector<index_t>& sorted_indices,
        index_t dimension,
        index_t stride,
        vector<index_t>::iterator first,
        vector<index_t>::iterator last,
        const vec3& period
    );

    /**
     * \brief Spatially sort a set of vertices in periodic space.
     * \param[in] nb_vertices total number of vertices, including the
     *  virtual periodic copies. This is 27 times the number of stored vertices.
     * \param[in] vertices pointer to the coordinates of the vertices
     * \param[in,out] sorted_indices a vector of vertex indices, sorted
     * \param[in] dimension number of vertices coordinates. Only 3 is supported.
     * \param[in] stride number of doubles between two consecutive vertices
     *  spatially on exit
     * \param[in] first index of the first element in \p sorted_indices
     *  to be sorted
     * \param[in] last one position past the index of the last element
     *  in \p sorted_indices to be sorted
     * \param[in] period the translation to be applied in periodic mode,
     *  translation vector is [ period , period , period ]
     */
    inline void Hilbert_sort_periodic(
        index_t nb_vertices, const double* vertices,
        vector<index_t>& sorted_indices,
        index_t dimension,
        index_t stride,
        vector<index_t>::iterator first,
        vector<index_t>::iterator last,
        double period = 1.0
    ) {
        Hilbert_sort_periodic(
            nb_vertices, vertices,
            sorted_indices,
            dimension,
            stride,
            first,
            last,
            vec3(period, period, period)
        );
    }
}

#endif
