# Homebrew formula for scowser
# To use: brew tap <your-username>/scowser && brew install scowser
#
# Publishing steps:
#   1. Create a GitHub repo: <your-username>/homebrew-scowser
#   2. Put this file in the repo as Formula/scowser.rb
#   3. Update the `url` and `sha256` fields to point to a release tarball
#   4. Users install with: brew tap <your-username>/scowser && brew install scowser

class scowser < Formula
  desc "Security-focused web browser with built-in ad blocking, DoH, and ephemeral sessions"
  homepage "https://github.com/YOUR_USERNAME/scowser"
  url "https://github.com/YOUR_USERNAME/scowser/archive/refs/tags/v0.0.12.tar.gz"
  sha256 "UPDATE_WITH_ACTUAL_SHA256"
  license "MIT"

  depends_on "cmake" => :build
  depends_on "qt@6"

  on_linux do
    depends_on "libseccomp"
    depends_on "pkg-config" => :build
  end

  def install
    args = %W[
      -DCMAKE_BUILD_TYPE=Release
      -DCMAKE_PREFIX_PATH=#{Formula["qt@6"].opt_prefix}
    ]

    system "cmake", "-S", ".", "-B", "build", *args, *std_cmake_args
    system "cmake", "--build", "build", "--parallel", ENV.make_jobs.to_s
    system "cmake", "--install", "build", "--prefix", prefix
  end

  def caveats
    <<~EOS
      scowser is a security-focused browser. By default:
        - All sessions are ephemeral (no data persists after exit)
        - DNS queries use DNS-over-HTTPS (Cloudflare)
        - Ads and trackers are blocked at the request level
        - All HTTP traffic is upgraded to HTTPS
    EOS
  end

  test do
    # Basic smoke test — verify binary runs and exits cleanly
    assert_match "scowser", shell_output("#{bin}/scowser --version 2>&1", 0)
  end
end
