#pragma once

#include "case_file.hpp"
#include "comparator.hpp"

#include <filesystem>

namespace proofcore::diff {

void write_failure_artifact(const std::filesystem::path& artifact_root,
                            const ProgramCase& program, const Mismatch& mismatch);

}  // namespace proofcore::diff
