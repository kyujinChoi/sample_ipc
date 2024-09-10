#ifndef PCL_UTILS_H
#define PCL_UTILS_H
#pragma once

#include "point_type.h"
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/common/transforms.h>
#include <pcl/filters/passthrough.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/impl/conditional_removal.hpp>
#include <pcl/filters/extract_indices.h>
#include <tf2/LinearMath/Quaternion.h>
#include "string.h"
inline double normalizeAngle(double angle) 
{
    while (angle >= M_PI / 2.0) 
        angle -= M_PI;
    while (angle <= -M_PI / 2.0) 
        angle += M_PI;
    return angle;
}
inline double calcDist3d(PointXYZIRL p1, PointXYZIRL p2)
{
    double dist = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) + (p1.z - p2.z) * (p1.z - p2.z));
    return dist;
}
inline double calcDist3d(pcl::PointXYZINormal p1, pcl::PointXYZINormal p2)
{
    double dist = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) + (p1.z - p2.z) * (p1.z - p2.z));
    return dist;
}
inline double calcDist3d(pcl::PointXYZ p1, pcl::PointXYZ p2)
{
    double dist = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) + (p1.z - p2.z) * (p1.z - p2.z));
    return dist;
}
inline void voxelize(
    const pcl::PointCloud<pcl::PointXYZ>::Ptr inCld,
    const pcl::PointCloud<pcl::PointXYZ>::Ptr outCld,
    float voxel_size)
{
  pcl::VoxelGrid<pcl::PointXYZ> voxel;
  voxel.setInputCloud(inCld);
  voxel.setLeafSize(voxel_size, voxel_size, voxel_size);
  voxel.filter(*outCld);
  return;
}
inline void voxelize(
    const pcl::PointCloud<pcl::PointXYZI>::Ptr inCld,
    const pcl::PointCloud<pcl::PointXYZI>::Ptr outCld,
    float voxel_size)
{
  pcl::VoxelGrid<pcl::PointXYZI> voxel;
  voxel.setInputCloud(inCld);
  voxel.setLeafSize(voxel_size, voxel_size, voxel_size);
  voxel.filter(*outCld);
  return;
}
inline void voxelize(
    const pcl::PointCloud<PointXYZIRL>::Ptr inCld,
    const pcl::PointCloud<PointXYZIRL>::Ptr outCld,
    float voxel_size)
{
  pcl::VoxelGrid<PointXYZIRL> voxel;
  voxel.setInputCloud(inCld);
  voxel.setLeafSize(voxel_size, voxel_size, voxel_size);
  voxel.filter(*outCld);
  return;
}
inline void rotatePointCloud(
    const pcl::PointCloud<pcl::PointXYZ>::Ptr inCld,
    const pcl::PointCloud<pcl::PointXYZ>::Ptr outCld,
    const Eigen::Matrix3d &R)
{
  Eigen::Matrix4d tf_r(Eigen::Matrix4d::Identity());
  tf_r.block(0, 0, 3, 3) = R; 
  pcl::transformPointCloud(*inCld, *outCld, tf_r);
  return;
}
inline void filterDist(pcl::PointCloud<PointXYZIRL>::Ptr cloud,
                      double thres, bool keep)
{
  pcl::PointIndices::Ptr inliers(new pcl::PointIndices());

  for (int i = 0; i < cloud->points.size(); ++i)
  {
    float distance = sqrt(cloud->points[i].x * cloud->points[i].x +
                          cloud->points[i].y * cloud->points[i].y +
                          cloud->points[i].z * cloud->points[i].z);
    if (distance <= thres)
    {
      inliers->indices.push_back(i);
    }
  }

  // 추출기 객체 생성
  pcl::ExtractIndices<PointXYZIRL> extract;
  extract.setInputCloud(cloud);
  extract.setIndices(inliers);
  extract.setNegative(!keep);
  extract.filter(*cloud);
  return;
}
inline void filterXYZ(pcl::PointCloud<PointXYZIRL>::Ptr cld,
                      pcl::PointXYZ min_p, pcl::PointXYZ max_p, std::string field, bool keep)
{
  pcl::ConditionalRemoval<PointXYZIRL> condrem;

  if(keep)
  {
    pcl::ConditionAnd<PointXYZIRL>::Ptr rangeCondAnd(new pcl::ConditionAnd<PointXYZIRL>());

    if(isContain(field,"x"))
    {
      rangeCondAnd->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("x", pcl::ComparisonOps::GT, min_p.x)));
      rangeCondAnd->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("x", pcl::ComparisonOps::LT, max_p.x)));
    }
    if(isContain(field,"y"))
    {
      rangeCondAnd->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("y", pcl::ComparisonOps::GT, min_p.y)));
      rangeCondAnd->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("y", pcl::ComparisonOps::LT, max_p.y)));
    }
    if(isContain(field,"z"))
    {
      rangeCondAnd->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("z", pcl::ComparisonOps::GT, min_p.z)));
      rangeCondAnd->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("z", pcl::ComparisonOps::LT, max_p.z)));
    }
    condrem.setCondition(rangeCondAnd);
  }
  else
  {
    pcl::ConditionOr<PointXYZIRL>::Ptr rangeCondOr(new pcl::ConditionOr<PointXYZIRL>());
    if(isContain(field,"x"))
    {
      rangeCondOr->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("x", pcl::ComparisonOps::LT, min_p.x)));
      rangeCondOr->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("x", pcl::ComparisonOps::GT, max_p.x)));
    }
    if (isContain(field, "y"))
    {
      rangeCondOr->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("y", pcl::ComparisonOps::LT, min_p.y)));
      rangeCondOr->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("y", pcl::ComparisonOps::GT, max_p.y)));
    }
    if (isContain(field, "z"))
    {
      rangeCondOr->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("z", pcl::ComparisonOps::LT, min_p.z)));
      rangeCondOr->addComparison(pcl::FieldComparison<PointXYZIRL>::ConstPtr(new pcl::FieldComparison<PointXYZIRL>("z", pcl::ComparisonOps::GT, max_p.z)));
    }
    condrem.setCondition(rangeCondOr);
  }
  condrem.setInputCloud(cld);
  condrem.setKeepOrganized(keep);
  condrem.filter(*cld);
  std::vector<int> indiceLet;
  pcl::removeNaNFromPointCloud(*cld, *cld, indiceLet);

  return;
}
inline void setROI(
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr &inCld,
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr &outCld,
    double x_min, double x_max, double y_min, double y_max, double z_min, double z_max)
{

    pcl::PassThrough<pcl::PointXYZINormal> pass;
    pass.setInputCloud(inCld);
    pass.setFilterFieldName("x");
    pass.setFilterLimits(x_min, x_max);
    pass.filter(*outCld);

    pass.setInputCloud(outCld);
    pass.setFilterFieldName("y");
    pass.setFilterLimits(y_min, y_max);
    pass.filter(*outCld);

    pass.setInputCloud(outCld);
    pass.setFilterFieldName("z");
    pass.setFilterLimits(z_min, z_max);
    pass.filter(*outCld);
    return;
}
inline Eigen::Matrix4d getTFMat(Eigen::Quaterniond q, Eigen::Vector3d t)
{
  Eigen::Matrix4d transform_Mat;
  transform_Mat << q.toRotationMatrix(), -q.toRotationMatrix() * t, 0, 0, 0, 1;
  return transform_Mat;
}
inline Eigen::Matrix4d getTFMat(double x, double y, double z, double roll, double pitch, double yaw)
{
  Eigen::AngleAxisd rollAngle(roll * M_PI / 180.0, Eigen::Vector3d::UnitX());
  Eigen::AngleAxisd pitchAngle(pitch * M_PI / 180.0, Eigen::Vector3d::UnitY());
  Eigen::AngleAxisd yawAngle(yaw * M_PI / 180.0, Eigen::Vector3d::UnitZ());

  Eigen::Quaterniond q = yawAngle * pitchAngle * rollAngle;
  Eigen::Matrix3d R = q.normalized().toRotationMatrix();
  Eigen::Matrix4d tf_r(Eigen::Matrix4d::Identity());
  tf_r.block(0, 0, 3, 3) = R;
  tf_r(0, 3) = x;
  tf_r(1, 3) = y;
  tf_r(2, 3) = z;
  
  return tf_r;
}
inline void tfPointCloud(
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr inCld,
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr outCld,
    double x, double y, double z, Eigen::Matrix3d R)
{
  // tf2::Quaternion euler2quat;
  // euler2quat.setRPY(roll, pitch, yaw);
  // Eigen::Quaternionf rotation_quat;
  // rotation_quat.x() = euler2quat.getX();
  // rotation_quat.y() = euler2quat.getY();
  // rotation_quat.z() = euler2quat.getZ();
  // rotation_quat.w() = euler2quat.getW();

  // Eigen::Matrix3f R = rotation_quat.normalized().toRotationMatrix();
  Eigen::Matrix4d tf_r(Eigen::Matrix4d::Identity());
  tf_r.block(0, 0, 3, 3) = R;
  tf_r(0, 3) = x;
  tf_r(1, 3) = y;
  tf_r(2, 3) = z;
  pcl::transformPointCloud(*inCld, *outCld, tf_r);   

  return;
}
inline void tfPointCloud(
    const pcl::PointCloud<pcl::PointXYZI>::Ptr inCld,
    const pcl::PointCloud<pcl::PointXYZI>::Ptr outCld,
    double x, double y, double z, double roll, double pitch, double yaw)
{
  Eigen::AngleAxisd rollAngle(roll * M_PI / 180.0, Eigen::Vector3d::UnitX());
  Eigen::AngleAxisd pitchAngle(pitch * M_PI / 180.0, Eigen::Vector3d::UnitY());
  Eigen::AngleAxisd yawAngle(yaw * M_PI / 180.0, Eigen::Vector3d::UnitZ());

  Eigen::Quaterniond q = yawAngle * pitchAngle * rollAngle;
  // Eigen::Matrix3d rotationMatrix = q.matrix();
  // tf2::Quaternion euler2quat;
  // euler2quat.setRPY(roll, pitch, yaw);
  // Eigen::Quaterniond rotation_quat;
  // rotation_quat.x() = euler2quat.getX();
  // rotation_quat.y() = euler2quat.getY();
  // rotation_quat.z() = euler2quat.getZ();
  // rotation_quat.w() = euler2quat.getW();

  Eigen::Matrix3d R = q.normalized().toRotationMatrix();
  Eigen::Matrix4d tf_r(Eigen::Matrix4d::Identity());
  tf_r.block(0, 0, 3, 3) = R;
  tf_r(0, 3) = x;
  tf_r(1, 3) = y;
  tf_r(2, 3) = z;
  if(inCld->points.size())
    pcl::transformPointCloud(*inCld, *outCld, tf_r);   
  return;
}
inline void tfPointCloud(
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr inCld,
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr outCld,
    double x, double y, double z, double roll, double pitch, double yaw)
{
  Eigen::AngleAxisd rollAngle(roll * M_PI / 180.0, Eigen::Vector3d::UnitX());
  Eigen::AngleAxisd pitchAngle(pitch * M_PI / 180.0, Eigen::Vector3d::UnitY());
  Eigen::AngleAxisd yawAngle(yaw * M_PI / 180.0, Eigen::Vector3d::UnitZ());

  Eigen::Quaterniond q = yawAngle * pitchAngle * rollAngle;
  // Eigen::Matrix3d rotationMatrix = q.matrix();
  // tf2::Quaternion euler2quat;
  // euler2quat.setRPY(roll, pitch, yaw);
  // Eigen::Quaterniond rotation_quat;
  // rotation_quat.x() = euler2quat.getX();
  // rotation_quat.y() = euler2quat.getY();
  // rotation_quat.z() = euler2quat.getZ();
  // rotation_quat.w() = euler2quat.getW();

  Eigen::Matrix3d R = q.normalized().toRotationMatrix();
  Eigen::Matrix4d tf_r(Eigen::Matrix4d::Identity());
  tf_r.block(0, 0, 3, 3) = R;
  tf_r(0, 3) = x;
  tf_r(1, 3) = y;
  tf_r(2, 3) = z;
  pcl::transformPointCloud(*inCld, *outCld, tf_r);   

  return;
}
inline void tfPointCloud(
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr inCld,
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr outCld,
    const Eigen::Matrix3d &tf)
{
  Eigen::Matrix4d tf_r(Eigen::Matrix4d::Identity());
  tf_r.block(0, 0, 3, 3) = tf;
  pcl::transformPointCloud(*inCld, *outCld, tf_r);
  return;
}

inline void ransacLine(
    const pcl::PointCloud<pcl::PointXYZ>::Ptr inCld,
    const pcl::ModelCoefficients::Ptr &coeff,
    const pcl::PointIndices::Ptr &inliers, Eigen::Vector3f axis,
    double eps_angle, double distance_threshold, int max_interations)
{
  pcl::SACSegmentation<pcl::PointXYZ> seg;
  seg.setOptimizeCoefficients(true);
  seg.setModelType(pcl::SACMODEL_PARALLEL_LINE);
  seg.setInputCloud(inCld);
  seg.setAxis(axis);
  seg.setEpsAngle(eps_angle * (M_PI / 180.0f));
  seg.setDistanceThreshold(distance_threshold);
  seg.setMaxIterations(max_interations);
  seg.segment(*inliers, *coeff);
  return;
}
inline void clustering(
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr inCld,
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr outCld,
    double tolerance, int cluster_min, int cluster_max, int threshold, double h_threshold)
{
  std::vector<pcl::PointIndices> cluster_indices;
  pcl::EuclideanClusterExtraction<pcl::PointXYZINormal> ec;
  ec.setClusterTolerance(tolerance);
  ec.setMinClusterSize(cluster_min);
  ec.setMaxClusterSize(cluster_max);
  ec.setInputCloud(inCld);
  ec.extract(cluster_indices);

  if (cluster_indices.empty())
  {
    return;
  }
  for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin();
       it != cluster_indices.end(); ++it)
  {
    if(it->indices.size() >= threshold)
    {
      pcl::PointCloud<pcl::PointXYZINormal>::Ptr tmp(new pcl::PointCloud<pcl::PointXYZINormal>);
      double height = 0.0;
      double min_z = 10.0;
      double max_z = -10.0;
      for (std::vector<int>::const_iterator pit = it->indices.begin();
           pit != it->indices.end(); ++pit)
      {
        if(pit == it->indices.begin())
        {
          tmp->push_back(inCld->points[*pit]);
          min_z = inCld->points[*pit].z;
          max_z = inCld->points[*pit].z;
          continue;
        }

        if(max_z - min_z >= h_threshold)
        {
          if(tmp->points.size() > 0)
          {
            *outCld += *tmp;
            tmp->clear();
          }
          outCld->push_back(inCld->points[*pit]);
          continue;
        }

        if(inCld->points[*pit].z < min_z)
          min_z = inCld->points[*pit].z;
        else if(inCld->points[*pit].z > max_z)
          max_z = inCld->points[*pit].z;
        
      }
    }
  }
  return;
}

inline void clustering(
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr inCld,
    const pcl::PointCloud<pcl::PointXYZINormal>::Ptr outCld,
    double tolerance, int cluster_min, int cluster_max)
{
  std::vector<pcl::PointIndices> cluster_indices;
  pcl::EuclideanClusterExtraction<pcl::PointXYZINormal> ec;
  ec.setClusterTolerance(tolerance);
  ec.setMinClusterSize(cluster_min);
  ec.setMaxClusterSize(cluster_max);
  ec.setInputCloud(inCld);
  ec.extract(cluster_indices);

  if (cluster_indices.empty())
  {
    return;
  }
  for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin();
       it != cluster_indices.end(); ++it)
  {
      for (std::vector<int>::const_iterator pit = it->indices.begin();
           pit != it->indices.end(); ++pit)
      {
        outCld->points.push_back(inCld->points[*pit]);
      }
  }
  return;
}
#endif