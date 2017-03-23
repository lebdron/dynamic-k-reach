#pragma once

#include <moderngpu/context.hxx>
#include <moderngpu/memory.hxx>

#include <moderngpu/kernel_scan.hxx>
#include <moderngpu/kernel_workcreate.hxx>     // dynamic work creation.

extern mgpu::standard_context_t mgpu_context;

struct workload_t {
  int count;
  int num_segments;
  mgpu::mem_t<int> segments;        // scanned sum of active vertex edge counts.
  mgpu::mem_t<int> edge_indices;    // indices into edges array.
};