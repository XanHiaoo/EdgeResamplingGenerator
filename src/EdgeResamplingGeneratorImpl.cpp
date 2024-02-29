#include "EdgeResamplingGeneratorImpl.h"

EdgeResamplingGeneratorImpl::EdgeResamplingGeneratorImpl(const cv::Mat& inputImage, const cv::Mat& maskImage)
    : srcImage_(inputImage.clone()), shapeMask_(maskImage.clone())
{
	resamplingWidth_ = static_cast<int>(std::min(inputImage.cols, inputImage.rows)/100);
    // 如果需要，可以添加其他初始化代码
}

const cv::Mat& EdgeResamplingGeneratorImpl::getSrcImage() const
{
    return srcImage_;
}

const cv::Mat& EdgeResamplingGeneratorImpl::getShapeMaskImage() const
{
    return shapeMask_;
}

const std::vector<cv::Mat>& EdgeResamplingGeneratorImpl::getResamplingImgs() const {
	return resamplingImgs_;
}

void EdgeResamplingGeneratorImpl::setSrcImage(const cv::Mat& inputImage)
{
    srcImage_ = inputImage.clone();
}

void EdgeResamplingGeneratorImpl::setShapeMaskImage(const cv::Mat& maskImage)
{
    shapeMask_ = maskImage.clone();
}

std::vector<std::vector<cv::Point>> EdgeResamplingGeneratorImpl::applyConnectedComponents()
{
    int num_components = cv::connectedComponents(shapeMask_, labels_);

    std::vector<std::vector<cv::Point>> components(num_components - 1);
    for (int x = 0; x < labels_.rows; ++x) {
        for (int y = 0; y < labels_.cols; ++y) {
            int label = labels_.at<int>(x, y);

            // Skip background label (label 0)
            if (label == 0) {
                continue;
            }
            components[label - 1].push_back(cv::Point(x, y));
        }
    }

    // 返回连通组件信息
    return components;
}

std::vector<std::vector<cv::Point>> EdgeResamplingGeneratorImpl::getBorderPoints(std::vector<std::vector<cv::Point>> connectedComponents) {
	borderLabels_ = shapeMask_.clone();

	// 创建一个向量用于边缘的点
	std::vector<std::vector<cv::Point>> border_points;
	// 遍历connectedComponents
	for (int i = 0; i < connectedComponents.size(); ++i) {
		// 创建一个向量用于存储当前connectedComponents中染色的点
		std::vector<cv::Point> component_border_points;
		// 遍历当前connectedComponents中的点
		for (const cv::Point& point : connectedComponents[i]) {
			int x = point.x;
			int y = point.y;

			std::vector<cv::Point> directions = {
				cv::Point(0, -1),  // 上
				cv::Point(0, 1),   // 下
				cv::Point(-1, 0),  // 左
				cv::Point(1, 0)    // 右
			};

			for (const cv::Point& dir : directions) {
				int new_x = x + dir.x;
				int new_y = y + dir.y;

				if (new_x >= 0 && new_x < borderLabels_.rows &&
					new_y >= 0 && new_y < borderLabels_.cols) {
					if (labels_.at<int>(new_x, new_y) != i + 1) {
						borderLabels_.at<uchar>(x, y) = 128;
						component_border_points.push_back(cv::Point(x, y));
						break;
					}
				}
			}
		}
		border_points.push_back(component_border_points);
	}
	return border_points;
}

std::vector<std::vector<cv::Point>> EdgeResamplingGeneratorImpl::getOrderBorderPoints(std::vector<std::vector<cv::Point>> borderPoints) {
	// 创建一个向量用于边缘的点
	std::vector<std::vector<cv::Point>> order_border_points;
	// 遍历components
	for (int i = 0; i < borderPoints.size(); ++i) {
		// 创建一个向量用于存储当前component中染色的点
		std::vector<cv::Point> component_order_border_points;
		// 遍历当前component中的点		
		int x = borderPoints[i][0].x;
		int y = borderPoints[i][0].y;

		std::vector<cv::Point> directions = {
			cv::Point(-1, 0),  // 上
			cv::Point(-1, 1),  //右上
			cv::Point(0, 1),   // 右
			cv::Point(1, 1),   //右下
			cv::Point(1, 0),   // 下
			cv::Point(1, -1),  //左下
			cv::Point(0, -1),  // 左
			cv::Point(-1,-1),  //左上
		};
		while (borderLabels_.at<uchar>(x, y) != i + 1 ) {
			if (cv::norm(cv::Point(x, y) - borderPoints[i][0]) <= 1 && component_order_border_points.size() > 2)
				break;
			borderLabels_.at<uchar>(x, y) = i + 1;
			component_order_border_points.push_back(cv::Point(x, y));
			for (const cv::Point& dir : directions) {
				int new_x = x + dir.x;
				int new_y = y + dir.y;

				if (new_x >= 0 && new_x < borderLabels_.rows &&
					new_y >= 0 && new_y < borderLabels_.cols &&
					std::find(borderPoints[i].begin(), borderPoints[i].end(), cv::Point(new_x, new_y)) != borderPoints[i].end() && borderLabels_.at<uchar>(new_x, new_y) == 128) {
					int count_ones = 0;
					int neighbors[][2] = { {0, -1}, {-1, 0}, {0, 1}, {1, 0} };

					for (int i = 0; i < 4; ++i) {
						int neighbor_x = new_x + neighbors[i][0];
						int neighbor_y = new_y + neighbors[i][1];

						// Check if the neighbor is within the image boundaries
						if (neighbor_x >= 0 && neighbor_x < borderLabels_.rows &&
							neighbor_y >= 0 && neighbor_y < borderLabels_.cols) {

							// Check if the neighbor pixel has a value of 1
							if (borderLabels_.at<uchar>(neighbor_x, neighbor_y) == 1) {
								count_ones++;
							}
						}
					}
					if (count_ones <= 1) {
						x = new_x;
						y = new_y;
						break;
					}
				}
			}
		}
		order_border_points.push_back(component_order_border_points);
	}
	return order_border_points;
}

std::vector<std::vector<cv::Point>> EdgeResamplingGeneratorImpl::getFilteredBorderPoints(std::vector<std::vector<cv::Point>> orderBorderPoints) {
	// 创建一个向量用于存储过滤后的点
	std::vector<std::vector<cv::Point>> filtered_order_border_points;

	// 遍历order_border_points中的每一组点
	for (const auto& component_order_border_points : orderBorderPoints) {
		// 创建一个向量用于存储符合条件的点
		std::vector<cv::Point> filtered_component_order_border_points;

		// 遍历当前component中的点，按顺序取出符合条件的点
		bool break_flag = false;
		for (int i = 0; i < component_order_border_points.size(); ++i) {		
			// 添加第一个点
			filtered_component_order_border_points.push_back(component_order_border_points[i]);
#ifdef _DEBUG
			cv::circle(imgShow_, cv::Point(component_order_border_points[i].y, component_order_border_points[i].x), 1, cv::Scalar(0, 255, 0), -1);
			cv::circle(imgShowOnMask_, cv::Point(component_order_border_points[i].y, component_order_border_points[i].x), 1, cv::Scalar(0, 255, 0), -1);
#endif
			// 遍历后续点，找出符合条件的点
			for (int j = i + 1; j < component_order_border_points.size(); ++j) {
				// 计算两点之间的距离
				double distance = cv::norm(component_order_border_points[j] - component_order_border_points[i]);

				// 判断是否符合条件
				if (distance > resamplingWidth_) {
					i = j;  // 更新i，跳过已加入的点
					break;
				}
				if (j == component_order_border_points.size() - 1)
					break_flag = true;
			}		
			if (break_flag)
				break;
		}

		// 将符合条件的点集加入结果向量
		filtered_order_border_points.push_back(filtered_component_order_border_points);
	}
	return filtered_order_border_points;
}

std::vector<cv::Point2f> EdgeResamplingGeneratorImpl::getNormals(const cv::Point& start, const cv::Point& end, int length) {
	length *= 2;
	std::vector<cv::Point2f> normals;
	cv::Point2f dirVector = end - start;
	cv::Point2f normalVector(-dirVector.y, dirVector.x);
	double norm = sqrt(normalVector.x * normalVector.x + normalVector.y * normalVector.y);
	normalVector.x /= norm;
	normalVector.y /= norm;

	cv::Point2f midpoint((start.x + end.x) / 2, (start.y + end.y) / 2);

	cv::Point2f normalStart = midpoint - length / 2 * normalVector;
	cv::Point2f normalEnd = midpoint + length / 2 * normalVector;

	if (normalEnd.x < 0 || normalEnd.x >= shapeMask_.cols ||
		normalEnd.y < 0 || normalEnd.y >= shapeMask_.rows ) {
		normals.push_back(midpoint);
		normals.push_back(cv::Point(normalStart));
	}
	else if (normalStart.x < 0 || normalStart.x >= shapeMask_.cols ||
		normalStart.y < 0 || normalStart.y >= shapeMask_.rows) {
		normals.push_back(midpoint);
		normals.push_back(cv::Point(normalEnd));
	}
	else if (shapeMask_.at<uchar>(static_cast<int>(normalEnd.y), static_cast<int>(normalEnd.x)) <
		shapeMask_.at<uchar>(static_cast<int>(normalStart.y), static_cast<int>(normalStart.x))) {
		normals.push_back(midpoint);
		normals.push_back(cv::Point(normalEnd));
	}
	else {
		normals.push_back(midpoint);
		normals.push_back(cv::Point(normalStart));
	}
	
	return normals;
}

std::vector<std::vector<std::vector<cv::Point2f>>> EdgeResamplingGeneratorImpl::getAllComponentNormals(std::vector<std::vector<cv::Point>> filteredBorderPoints) {
	std::vector<std::vector<std::vector<cv::Point2f>>> all_component_normals;
	for (const auto& component_points : filteredBorderPoints) {
		std::vector<std::vector<cv::Point2f>> component_normals;		
		for (size_t i = 1; i < component_points.size(); ++i) {
			std::vector<cv::Point2f> normals = getNormals(cv::Point2f(component_points[i - 1].y, component_points[i - 1].x),cv::Point2f(component_points[i].y, component_points[i].x), resamplingWidth_);
			component_normals.push_back(normals);
		}
		all_component_normals.push_back(component_normals);
	}
	return all_component_normals;
}

void EdgeResamplingGeneratorImpl::resamplingImgs(std::vector<std::vector<std::vector<cv::Point2f>>> allComponentNormals) {
	// 创建一个向量用于存储所有结果的列拼接
	std::vector<cv::Mat> resampling_imgs;
	for (const auto& component_normals : allComponentNormals) {
		cv::Mat component_img;
		if (component_normals.size() != 0) {
#ifdef _DEBUG
			cv::line(imgShow_, component_normals[0][0], component_normals[0][1], cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
			cv::line(imgShowOnMask_, component_normals[0][0], component_normals[0][1], cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
#endif
		}
		for (size_t i = 0; i < component_normals.size(); i++) {
			if (i + 1 >= component_normals.size()) {
				break;
			}		
#ifdef _DEBUG
			cv::line(imgShow_, component_normals[i+1][0], component_normals[i+1][1], cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
			cv::line(imgShowOnMask_, component_normals[i + 1][0], component_normals[i + 1][1], cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
#endif
			// Extracting points from components
			cv::Point2f inputQuad[4] = {
				component_normals[i][0],
				component_normals[i + 1][0],
				component_normals[i + 1][1],
				component_normals[i][1]
			};

			// Convert the array to a cv::Mat
			cv::Mat inputQuadMat = cv::Mat(4, 1, CV_32FC2, inputQuad);

			// 计算输入梯形区域的最小矩形框
			cv::Rect boundingBox = cv::boundingRect(inputQuadMat);

			// 左上角相对于 boundingBox 左上角的偏移
			cv::Point2f offset = boundingBox.tl();
			// 创建一个黑色背景图像，大小与梯形区域大小相匹配
			cv::Mat background = cv::Mat::zeros(boundingBox.size(), srcImage_.type());

			// 提取梯形区域
			cv::Mat roi = srcImage_(boundingBox);

			std::vector<cv::Point2f> srcPoints;
			for (int j = 0; j < 4; ++j) {
				srcPoints.push_back({ inputQuad[j].x - offset.x, inputQuad[j].y - offset.y });
			}

			// 假设目标矩形的大小
			cv::Size targetSize(roi.cols, roi.rows);

			// 定义目标矩形的四个顶点坐标
			std::vector<cv::Point2f> dstPoints;
			dstPoints.push_back(cv::Point2f(0, 0));
			dstPoints.push_back(cv::Point2f(targetSize.width - 1, 0));
			dstPoints.push_back(cv::Point2f(targetSize.width - 1, targetSize.height - 1));
			dstPoints.push_back(cv::Point2f(0, targetSize.height - 1));

			// 计算透视变换矩阵
			cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);

			// 应用透视变换
			cv::Mat result;
			cv::warpPerspective(roi, result, perspectiveMatrix, targetSize);

			// 缩放 result
			cv::Mat scaled_result;
			cv::resize(result, scaled_result, cv::Size(result.cols * resamplingWidth_ / result.rows, resamplingWidth_));

			if (component_img.empty()) {
				component_img.push_back(scaled_result);
			}
			else {
				// 进行列拼接
				cv::hconcat(component_img, scaled_result, component_img);
			}
		}
		resampling_imgs.push_back(component_img);
	}
	resamplingImgs_ = resampling_imgs;
}

void EdgeResamplingGeneratorImpl::getResult() {
#ifdef _DEBUG
	imgShow_ = srcImage_.clone();
	imgShowOnMask_ = shapeMask_.clone();
	cv::cvtColor(imgShow_, imgShow_, cv::COLOR_GRAY2BGR);
	cv::cvtColor(imgShowOnMask_, imgShowOnMask_, cv::COLOR_GRAY2BGR);
#endif
	std::vector<std::vector<cv::Point>> connected_components = applyConnectedComponents();
	std::vector<std::vector<cv::Point>> border_points = getBorderPoints(connected_components);
	std::vector<std::vector<cv::Point>> order_border_points =  getOrderBorderPoints(border_points);
	std::vector<std::vector<cv::Point>> filtered_border_points = getFilteredBorderPoints(order_border_points);
	std::vector<std::vector<std::vector<cv::Point2f>>> all_component_normals = getAllComponentNormals(filtered_border_points);
	resamplingImgs(all_component_normals);
}