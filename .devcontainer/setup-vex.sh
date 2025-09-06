#!/bin/bash

echo "🚀 Setting up Vexanium Development Environment..."

# Update system
sudo apt update && sudo apt upgrade -y

# Install dependencies
sudo apt install -y wget curl build-essential cmake git

# Download dan install VEX.CDT
echo "📦 Installing VEX.CDT..."
cd /tmp
wget https://vexanium.s3-ap-southeast-1.amazonaws.com/dl/vex.cdt.amd64.deb
sudo apt install -y ./vex.cdt.amd64.deb

# Install VEX CPP compiler tambahan
echo "📦 Installing VEXCPP..."
wget https://vexgift.s3-ap-southeast-1.amazonaws.com/dl/vex.cdt.add.amd64.deb
sudo apt install -y ./vex.cdt.add.amd64.deb

# Verify installation
echo "✅ Verifying installation..."
vexcpp --version
eosio-cpp --version

# Setup workspace
cd /workspaces/vexanium-contracts
mkdir -p contracts/sendvexjalil/{include/sendvexjalil,src}
mkdir -p build scripts

echo "🎉 Vexanium development environment ready!"
echo "📝 You can now compile contracts with: vexcpp or eosio-cpp"
