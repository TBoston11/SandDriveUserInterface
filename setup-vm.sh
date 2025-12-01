#!/bin/bash
set -e

echo "Setting up sdui-debian VM..."

# Create VM storage
sudo mkdir -p /var/lib/libvirt/images/sdui
cd /var/lib/libvirt/images/sdui

# Download Debian cloud image if needed
if [ ! -f debian-12-generic-amd64.qcow2 ]; then
    echo "Downloading Debian cloud image..."
    sudo wget -O debian-12-generic-amd64.qcow2 \
        https://cloud.debian.org/images/cloud/bookworm/latest/debian-12-generic-amd64.qcow2
fi

# Create cloud-init config
cat > user-data <<'EOF'
#cloud-config
users:
  - name: debian
    plain_text_passwd: debian
    lock_passwd: false
    ssh_pwauth: true
    sudo: ALL=(ALL) NOPASSWD:ALL
    shell: /bin/bash
chpasswd:
  list: |
    debian:debian
  expire: false
ssh_authorized_keys: []
disable_root: false
package_update: true
packages:
  - openssh-server
runcmd:
  - systemctl enable ssh
  - systemctl start ssh
EOF

cat > meta-data <<'EOF'
instance-id: sdui-vm-001
local-hostname: sdui-vm
EOF

# Create seed image
sudo cloud-localds seed.img user-data meta-data

# Create working disk
sudo qemu-img create -f qcow2 -F qcow2 -b debian-12-generic-amd64.qcow2 sdui-debian.qcow2 10G

# Fix permissions
sudo chown -R libvirt-qemu:libvirt-qemu /var/lib/libvirt/images/sdui/

# Create VM
virt-install \
  --connect qemu:///system \
  --name sdui-debian \
  --memory 2048 \
  --vcpus 2 \
  --disk path=/var/lib/libvirt/images/sdui/sdui-debian.qcow2,format=qcow2 \
  --disk path=/var/lib/libvirt/images/sdui/seed.img,format=raw \
  --network network=default \
  --graphics none \
  --os-variant debian12 \
  --import \
  --noautoconsole

echo "VM created. Waiting 60 seconds for cloud-init..."
sleep 60

echo "Checking VM IP..."
virsh --connect qemu:///system domifaddr sdui-debian

echo ""
echo "VM setup complete!"
echo "Login: debian / debian"
echo ""
echo "To connect:"
echo "  virsh --connect qemu:///system console sdui-debian"
echo "  (Press Ctrl+] to exit console)"
