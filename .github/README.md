# GitHub Actions Workflows

This directory contains GitHub Actions workflows for automated build checks and continuous integration.

## Workflows

### 1. Build Check (`build.yml`)

**Triggers:** Push/PR to main or develop branches

- Builds the firmware using PlatformIO
- Caches dependencies for faster builds
- Uploads build artifacts
- Matrix strategy ready for multiple environments

### 2. Library Dependency Check (`dependencies.yml`)

**Triggers:** Push/PR to main or develop branches, weekly schedule

- Updates library index
- Checks for outdated libraries
- Verifies all dependencies can be resolved
- Runs weekly to catch library updates

### 3. Multi-Environment Build (`multi-build.yml`)

**Triggers:** Push/PR to main branch

- Builds firmware for multiple target environments
- Checks binary size limits
- Caches build dependencies efficiently
- Uploads firmware artifacts with unique names

### 4. Release Build (`release.yml`)

**Triggers:** Git tags starting with 'v' (e.g., v1.0.0)

- Creates release builds for tagged versions
- Generates checksums for verification
- Creates GitHub releases with firmware binaries
- Includes build information and documentation

## Usage

### Setting up Branch Protection

To require these checks to pass before merging:

1. Go to your repository settings
2. Navigate to "Branches"
3. Add a branch protection rule for `main`
4. Enable "Require status checks to pass before merging"
5. Select the following checks:
   - `build`
   - `dependency-check`
   - `build-matrix`

### Creating a Release

To create a new release:

1. Create and push a git tag:

   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```

2. The release workflow will automatically:
   - Build the firmware
   - Create a GitHub release
   - Upload firmware binaries
   - Generate checksums

## Customization

### Adding New Environments

To add support for additional ESP32 variants, edit `multi-build.yml`:

```yaml
strategy:
  matrix:
    include:
      - environment: esp32dev
        platform: espressif32
        board: esp32dev
      - environment: esp32-s3
        platform: espressif32
        board: esp32-s3-devkitc-1
```

### Adjusting Binary Size Limits

Edit the `max_size` value in `multi-build.yml`:

```yaml
max_size=1200000 # Adjust based on your partition scheme
```

## Artifacts

Build artifacts are automatically uploaded and include:

- Firmware binaries (`.bin`)
- Debug symbols (`.elf`)
- Build logs
- Checksums

Artifacts are retained for:

- Regular builds: 14-30 days
- Release builds: Permanent (via GitHub Releases)
