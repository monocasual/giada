#!/usr/bin/env bash
#
# Creates source tarballs for giada in the form of
# 'giada-x.x.x-src.tar.gz' and optionally detached PGP signatures
# for the created file of the form 'giada-x.x.x-src.tar.gz.asc'.
# If the environment variable BUILD_DIR is provided, the files will be moved to
# $BUILD_DIR/, else to the location of this script (the repository folder).
#
# Requirements:
# - git
# - tar
# - a writable (user) /tmp folder for mktemp
# - gnupg >= 2.0.0 (if source tarball signing is requested)
# - a valid PGP signing key in the keyring (if source tarball signing is
# requested)

set -euo pipefail

get_absolute_path() {
    cd "$(dirname "$1")" && pwd -P
}

validate_project_tag() {
    if ! git ls-remote -t "${upstream}"| grep -e "${version}$" > /dev/null; then
        echo "The tag '$version' could not be found in upstream repository (${upstream})."
        exit 1
    fi
}

checkout_project() {
    echo "Cloning project below working directory ${working_dir}"
    cd "$working_dir"
    git clone "$upstream" --branch "$version" \
                          --single-branch \
                          --depth=1 \
                          --recurse-submodules \
                          --shallow-submodules \
                          "${output_name}"
}

clean_sources() {
    cd "${working_dir}/${output_name}"
    echo "Removing unneeded files and folders..."
    rm -rfv .git* \
            .travis* \
            create_source_tarball.sh
}

compress_sources() {
    cd "${working_dir}"
    tar cfz "${output_name}.tar.gz" "${output_name}"
}

move_sources() {
    cd "${working_dir}"
    mv -v "${output_name}.tar.gz" "${output_dir}/"
}

sign_sources() {
    cd "${output_dir}"
    gpg --detach-sign \
        -u "${signer}" \
        -o "${output_name}.tar.gz.asc" \
        "${output_name}.tar.gz"
}

cleanup_working_dir() {
    echo "Removing working directory: ${working_dir}"
    rm -rf "${working_dir}"
}

print_help() {
    echo "Usage: $0 -v <version tag> -s <signature email or key ID>"
    exit 1
}

if [ -n "${BUILD_DIR:-}" ]; then
    echo "Build dir provided: ${BUILD_DIR}"
    output_dir="${BUILD_DIR}/"
    mkdir -p "${output_dir}"
else
    output_dir="$(get_absolute_path "$0")"
fi

upstream="https://github.com/monocasual/giada"
package_name="giada"
working_dir="$(mktemp -d)"
version="$(date '+%Y-%m-%d')"
output_version=""
output_name=""
signer=""
signature=0

# remove the working directory, no matter what
trap cleanup_working_dir EXIT

if [ ${#@} -gt 0 ]; then
    while getopts 'hv:s:' flag; do
        case "${flag}" in
            h) print_help
                ;;
            s) signer=$OPTARG
                signature=1
                ;;
            v) version=$OPTARG
                output_version="${version//v}"
                ;;
            *)
                echo "Error! Try '${0} -h'."
                exit 1
                ;;
        esac
    done
else
    print_help
fi

output_name="${package_name}-${output_version}-src"
validate_project_tag
checkout_project
clean_sources
compress_sources
move_sources
if [ $signature -eq 1 ]; then
    sign_sources
fi

exit 0

# vim:set ts=4 sw=4 et:
