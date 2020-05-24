#include <iostream>
#include <fstream>
#include <sstream>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>

bool create_bucket(const Aws::String &bucket_name,
    const Aws::S3::Model::BucketLocationConstraint &region = Aws::S3::Model::BucketLocationConstraint::us_east_1)
{
    // Set up the request
    Aws::S3::Model::CreateBucketRequest request;
    request.SetBucket(bucket_name);

    // Is the region other than us-east-1 (N. Virginia)?
    if (region != Aws::S3::Model::BucketLocationConstraint::us_east_1)
    {
        // Specify the region as a location constraint
        Aws::S3::Model::CreateBucketConfiguration bucket_config;
        bucket_config.SetLocationConstraint(region);
        request.SetCreateBucketConfiguration(bucket_config);
    }

    // Create the bucket
    Aws::S3::S3Client s3_client;
    auto outcome = s3_client.CreateBucket(request);
    if (!outcome.IsSuccess())
    {
        auto err = outcome.GetError();
        std::cout << "ERROR: CreateBucket: " << 
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        return false;
    }
    return true;
}

bool upload_object(const Aws::String& s3_bucket_name, 
    const Aws::String& s3_object_name, 
    const std::string& file_name, 
    const Aws::String& region = "")
{
    // Set up request
    // snippet-start:[s3.cpp.put_object.code]
    Aws::Client::ClientConfiguration clientConfig;
    Aws::S3::S3Client s3_client(clientConfig);
    Aws::S3::Model::PutObjectRequest object_request;

    object_request.SetBucket(s3_bucket_name);
    object_request.SetKey(s3_object_name);
    const std::shared_ptr<Aws::IOStream> input_data = 
        Aws::MakeShared<Aws::FStream>("SampleAllocationTag", 
                                      file_name.c_str(), 
                                      std::ios_base::in | std::ios_base::binary);
    object_request.SetBody(input_data);

    // Put the object
    auto put_object_outcome = s3_client.PutObject(object_request);
    if (!put_object_outcome.IsSuccess()) {
        auto error = put_object_outcome.GetError();
        std::cout << "ERROR: " << error.GetExceptionName() << ": " 
            << error.GetMessage() << std::endl;
        return false;
    }
    return true;
    // snippet-end:[s3.cpp.put_object.code]
}

void delete_object(const Aws::String& s3_bucket_name, const Aws::String& s3_object_name)
{
    Aws::S3::S3Client s3_client;

    Aws::S3::Model::DeleteObjectRequest object_request;
    object_request.WithBucket(s3_bucket_name).WithKey(s3_object_name);

    auto delete_object_outcome = s3_client.DeleteObject(object_request);

    if (!delete_object_outcome.IsSuccess())
    {
        std::cout << "DeleteObject error: " <<
            delete_object_outcome.GetError().GetExceptionName() << " " <<
            delete_object_outcome.GetError().GetMessage() << std::endl;
    }
}

std::string get_object(const Aws::String& s3_bucket_name, const Aws::String& s3_object_name)
{
    // Set up the request
    Aws::S3::S3Client s3_client;
    Aws::S3::Model::GetObjectRequest object_request;
    object_request.SetBucket(s3_bucket_name);
    object_request.SetKey(s3_object_name);

    // Get the object
    auto get_object_outcome = s3_client.GetObject(object_request);
    if (get_object_outcome.IsSuccess())
    {
        // Get an Aws::IOStream reference to the retrieved file
        auto &retrieved_file = get_object_outcome.GetResultWithOwnership().GetBody();

        std::stringstream ss;
        ss << retrieved_file.rdbuf();
        return ss.str();
    }
    else
    {
        auto error = get_object_outcome.GetError();
        std::cout << "ERROR: " << error.GetExceptionName() << ": " 
            << error.GetMessage() << std::endl;
        return "";
    }
}